//
// Created by root on 5/11/17.
//

#include <string.h>
#include "lgecnradapter.h"

//LGECNR_PERIOD = 16 * GST_MSECOND
//period_size = 8000Hz * 0.016s * 2Bytes
//LGECNR_PERIOD_SIZE 256

#ifndef DISABLE_LGECNR_ENGINE
#include <ringbuffer.h>
#include <tseFilter.h>
#include <tseFilter_priv.h>

#define ECNR_CONFIG_PATH "/etc/ecnr/config/"
#define ECNR_LIB_FILE_FULLPATH "/app/lib/libtse.so"
#define ECNR_TELEMATICS_MODE 3
#endif

GST_DEBUG_CATEGORY_STATIC(gst_lgecnradapter_debug_category);
#define GST_CAT_DEFAULT gst_lgecnradapter_debug_category

static const gchar ZERO_FRAME[10] = {0x00};

static inline GstClockTime AdjustClockTime(GstClockTime time) {
    if (!GST_CLOCK_TIME_IS_VALID(time)) return GST_CLOCK_TIME_NONE;
    else return (GST_MSECOND - (time % GST_MSECOND));
}


LgEcnrAdapter::LgEcnrAdapter() :
        m_RefTime(GST_CLOCK_TIME_NONE),
        m_IsFirstRecvBuffer(1),m_IsFirstMicBuffer(1),
        m_RecvStreamLatency(0), m_RecvStreamDelay(0),
        m_PeriodTime(GST_CLOCK_TIME_NONE),m_PeriodSize(0),
        m_rate(0),m_channels(0),m_bpf(0),m_volume(15),
        m_MicAdjustTime(GST_CLOCK_TIME_NONE), m_RecvAdjustTime(GST_CLOCK_TIME_NONE),
        m_TimerRunning(FALSE),m_clock(NULL), m_ClockId(0),
		m_MicInBufferSize(0), m_RecvInBufferSize(0) , m_lgecnr(NULL), m_lgecnrprobe(NULL) {

    GST_DEBUG_CATEGORY_INIT(gst_lgecnradapter_debug_category, "lgecnradapter", 0, "debug category for lgecnr adapter class");
    g_mutex_init(&m_MicInLock);
    g_mutex_init(&m_MicOutLock);
    g_mutex_init(&m_RecvInLock);
    g_mutex_init(&m_RecvOutLock);

    m_MicInAdapter = gst_adapter_new();
    m_RecvInAdapter = gst_adapter_new();
    m_MicOutAdapter = gst_adapter_new();
    m_RecvOutAdapter = gst_adapter_new();

#ifndef DISABLE_LGECNR_ENGINE
	m_TseCtx = NULL;
#endif

}

LgEcnrAdapter::~LgEcnrAdapter() {
    g_message("%s",__PRETTY_FUNCTION__);
    g_mutex_clear(&m_MicInLock);
    g_mutex_clear(&m_MicOutLock);
    g_mutex_clear(&m_RecvInLock);
    g_mutex_clear(&m_RecvOutLock);

    gst_adapter_clear (m_MicInAdapter);
    g_object_unref (m_MicInAdapter);
    gst_adapter_clear (m_MicOutAdapter);
    g_object_unref (m_MicOutAdapter);

    gst_adapter_clear (m_RecvInAdapter);
    g_object_unref (m_RecvInAdapter);
    gst_adapter_clear (m_RecvOutAdapter);
    g_object_unref (m_RecvOutAdapter);

    g_message("%s done",__PRETTY_FUNCTION__);
}



gboolean LgEcnrAdapter::PushMicBuffer(GstBuffer *buffer) {

    buffer = gst_buffer_make_writable(buffer);

    if (GST_BUFFER_IS_DISCONT(buffer)) {
        MICIN_LOCK();
        gst_adapter_clear(m_MicInAdapter);
        m_MicAdjustTime = AdjustClockTime (GST_BUFFER_PTS(buffer));
        MICIN_UNLOCK();
    }

    gst_buffer_ref(buffer);
    MICIN_LOCK();
    gst_adapter_push (m_MicInAdapter, buffer);
    MICIN_UNLOCK();

    StartTimer();

    guint64 distance;
    GstClockTime pts;
    pts = gst_adapter_prev_pts(m_MicInAdapter, &distance);
    pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);
    GST_DEBUG ("MicInAdapter pts = %" GST_TIME_FORMAT ", buffer pts = %" G_GUINT64_FORMAT " , duration = %" G_GUINT64_FORMAT , GST_TIME_ARGS(pts), GST_BUFFER_PTS(buffer), GST_BUFFER_DURATION(buffer));

    return TRUE;
}

static gboolean timer_expired(GstClock *clock, GstClockTime time,GstClockID id, gpointer user_data) {
    LgEcnrAdapter *adapter = (LgEcnrAdapter *)user_data;
    GST_DEBUG("****** expired (Next Reference Time %" GST_TIME_FORMAT ")", GST_TIME_ARGS(adapter->m_RefTime));
    adapter->process();
    return TRUE;
}

gboolean LgEcnrAdapter::PushRecvBuffer(GstBuffer *buffer) {

    buffer = gst_buffer_make_writable(buffer);

    if (GST_BUFFER_IS_DISCONT(buffer)) {
        RECVIN_LOCK();
        gst_adapter_clear(m_RecvInAdapter);
        m_RecvAdjustTime = AdjustClockTime (GST_BUFFER_PTS(buffer));
        RECVIN_UNLOCK();
    }

    gst_buffer_ref(buffer);
    RECVIN_LOCK();
    gst_adapter_push (m_RecvInAdapter, buffer);
    RECVIN_UNLOCK();

    guint64 distance;
    GstClockTime pts;
    pts = gst_adapter_prev_pts(m_RecvInAdapter, &distance);
    pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);

    GST_DEBUG ("RecvInAdapter pts = %" GST_TIME_FORMAT ", buffer pts = %" G_GUINT64_FORMAT " , duration = %" G_GUINT64_FORMAT , GST_TIME_ARGS(pts), GST_BUFFER_PTS(buffer), GST_BUFFER_DURATION(buffer));

    StartTimer();

    return TRUE;
}


void LgEcnrAdapter::StartTimer() {
    GstClockTime ref_time = GST_CLOCK_TIME_NONE;

    if (m_TimerRunning) return;
    if (!m_MicInAdapter || !m_RecvInAdapter || !gst_adapter_available(m_MicInAdapter) || !gst_adapter_available(m_RecvInAdapter)) return;

    GstClockTime micin_adapter_pts;
    GstClockTime recvin_adapter_pts;
    GstClockTime ref_time1, ref_time2;
    guint64 distance;

    MICIN_LOCK();
    RECVIN_LOCK();

    micin_adapter_pts = gst_adapter_prev_pts (m_MicInAdapter, &distance);
    micin_adapter_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);
    recvin_adapter_pts = gst_adapter_prev_pts (m_RecvInAdapter, &distance);
    recvin_adapter_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);

    if (!GST_CLOCK_TIME_IS_VALID(m_MicAdjustTime)) m_MicAdjustTime = AdjustClockTime(micin_adapter_pts);
    if (!GST_CLOCK_TIME_IS_VALID(m_RecvAdjustTime)) m_RecvAdjustTime = AdjustClockTime(recvin_adapter_pts);

    ref_time1 = MicToRefTime(micin_adapter_pts);
    ref_time2 = RecvToRefTime(recvin_adapter_pts);

    if (GST_CLOCK_TIME_IS_VALID(ref_time1) && GST_CLOCK_TIME_IS_VALID(ref_time2)) {
        guint64 micin_avail = 0;
        guint64 recvin_avail = 0;

        ref_time = MAX(ref_time1, ref_time2);

        FlushAdapterUntil(m_MicInAdapter, RefToMicTime(ref_time), &micin_avail);
        FlushAdapterUntil(m_RecvInAdapter, RefToRecvTime(ref_time), &recvin_avail);
        MICIN_UNLOCK();
        RECVIN_UNLOCK();
        if (micin_avail < m_MicInBufferSize || recvin_avail < m_RecvInBufferSize) { //not enouch data in MicIn or RecvIn adapter
            return;
        }
        m_RefTime = ref_time;
        GST_INFO("Reference Time = %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(ref_time));
    } else {
        MICIN_UNLOCK();
        RECVIN_UNLOCK();
        return;
    }

    if (g_atomic_int_compare_and_exchange(&m_TimerRunning, (gint)FALSE, (gint)TRUE)) {
        m_clock = gst_system_clock_obtain();
        m_ClockId = gst_clock_new_periodic_id (m_clock, gst_clock_get_time(m_clock), m_PeriodTime);
        gst_clock_id_wait_async (m_ClockId, timer_expired, this, NULL);
    }
}


void LgEcnrAdapter::StopTimer() {
	GST_INFO("StopTimer ");
	if(m_ClockId)
	{
		gst_clock_id_unschedule (m_ClockId);
		gst_clock_id_unref (m_ClockId);
	}
	if (m_clock) gst_object_unref(m_clock);
	SetObjects(NULL, NULL);
#ifndef DISABLE_LGECNR_ENGINE
	if(m_TseCtx != NULL)
	{
		GST_DEBUG(" LGTSE_FilterDestroy!!");
		LGTSE_FilterDestroy(m_TseCtx);
		m_TseCtx = NULL;
	}
#endif
}

void LgEcnrAdapter::process() {
    GstClockTime ref_time = GST_CLOCK_TIME_NONE;
    gchar micin_buf[m_PeriodSize] = {0};
    gchar recvin_buf[m_PeriodSize] = {0};
    GstBuffer *micout_buf;
    GstMapInfo  micout_minfo;
    GstBuffer *recvout_buf;
    GstMapInfo  recvout_minfo;

    MICIN_LOCK();
    RECVIN_LOCK();

    ref_time = m_RefTime;

    if (!GST_CLOCK_TIME_IS_VALID(ref_time)) goto _process_failed;

    GST_DEBUG("Ref-Time = %" GST_TIME_FORMAT " ,Mic-Time = %" GST_TIME_FORMAT " ,Recv-Time = %" GST_TIME_FORMAT,
              GST_TIME_ARGS(ref_time), GST_TIME_ARGS(RefToMicTime(ref_time)), GST_TIME_ARGS(RefToRecvTime(ref_time)));

    guint64 distance;
    GstClockTime start_pts;
    GstClockTime stop_pts;
    gsize remains;
    start_pts = gst_adapter_prev_pts(m_MicInAdapter, &distance);
    start_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);
    remains = gst_adapter_available(m_MicInAdapter);
    stop_pts = start_pts + gst_util_uint64_scale_int(remains/m_bpf, GST_SECOND, m_rate);
    GST_DEBUG("m_MicInAdapter start pts = %" GST_TIME_FORMAT " , end pts = %" GST_TIME_FORMAT, GST_TIME_ARGS(start_pts), GST_TIME_ARGS(stop_pts));


    start_pts = gst_adapter_prev_pts(m_RecvInAdapter, &distance);
    start_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);
    remains = gst_adapter_available(m_RecvInAdapter);
    stop_pts = start_pts + gst_util_uint64_scale_int(remains/m_bpf, GST_SECOND, m_rate);
    GST_DEBUG("m_RecvInAdapter start pts = %" GST_TIME_FORMAT " , end pts = %" GST_TIME_FORMAT, GST_TIME_ARGS(start_pts), GST_TIME_ARGS(stop_pts));

    ReadFromAdapter(m_MicInAdapter, RefToMicTime(ref_time), micin_buf, NULL, NULL);
    ReadFromAdapter(m_RecvInAdapter, RefToRecvTime(ref_time), recvin_buf, NULL, NULL);
    MICIN_UNLOCK();
    RECVIN_UNLOCK();

    // pass read data to LG ECNR ENGINE
    micout_buf = gst_buffer_new_allocate(NULL, m_PeriodSize, NULL);
    recvout_buf = gst_buffer_new_allocate(NULL, m_PeriodSize, NULL);
    gst_buffer_map(micout_buf, &micout_minfo, GST_MAP_WRITE);
    gst_buffer_map(recvout_buf, &recvout_minfo, GST_MAP_WRITE);

#ifdef DISABLE_LGECNR_ENGINE
    memcpy(micout_minfo.data, micin_buf, m_PeriodSize);
    memcpy(recvout_minfo.data, recvin_buf, m_PeriodSize);
#else
    LGTSE_FilterProcess(m_TseCtx, (char *) micin_buf, (char *) micout_minfo.data, (char *) recvin_buf,
                        (char *) recvout_minfo.data, &m_volume);
#endif
    gst_buffer_unmap(micout_buf, &micout_minfo);
    gst_buffer_unmap(recvout_buf, &recvout_minfo);

    // delay m_PeriodTime(16ms) for micout and recvout
    GST_BUFFER_PTS (micout_buf) = RefToMicTime(ref_time);
    GST_BUFFER_DURATION (micout_buf) = m_PeriodTime;
    GST_BUFFER_PTS (recvout_buf) = RefToRecvTime(ref_time);
    GST_BUFFER_DURATION (recvout_buf) = m_PeriodTime;

    GST_DEBUG("Directly push to src pads");
	if (m_lgecnrprobe)
		gst_pad_push (m_lgecnrprobe->srcpad, recvout_buf);
	if (m_lgecnr)
		gst_pad_push (m_lgecnr->srcpad, micout_buf);

    m_RefTime = ref_time + m_PeriodTime;
    return;

    _process_failed:
    MICIN_UNLOCK();
    RECVIN_UNLOCK();
}

void LgEcnrAdapter::FlushAdapterUntil(GstAdapter *adapter, GstClockTime time, guint64 *remains) {
    GstClockTime adapter_time;
    GstClockTimeDiff diff;
    guint64 distance;
    gsize adapter_avail = 0;
    guint64 discard_size = 0;

    adapter_time = gst_adapter_prev_pts(adapter, &distance);
    adapter_time += gst_util_uint64_scale_int (distance / m_bpf, GST_SECOND, m_rate);

    adapter_avail = gst_adapter_available (adapter);

    if (adapter_time < time) {
        diff = GST_CLOCK_DIFF (adapter_time, time);
        discard_size = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        discard_size = MIN(discard_size, adapter_avail);
    }
    GST_DEBUG("discard size = %" G_GUINT64_FORMAT , discard_size);

    gst_adapter_flush(adapter, discard_size);
    if (remains) *remains = gst_adapter_available(adapter);
}

/*
 * Caller MUST get a lock for the adapter before this function.
 */
gboolean LgEcnrAdapter::ReadFromAdapter(GstAdapter *adapter, GstClockTime start_time, gchar *out_buf, guint64 *skip_len,
                                        guint64 *read_len) {
    GstClockTime adapter_time;
    GstClockTimeDiff diff;
    guint64 distance;
    gsize adapter_avail;
    guint64 skip = 0;
    guint64 offset = 0;
    guint64 copy_size;

    adapter_time = gst_adapter_prev_pts(adapter, &distance);
    adapter_time += gst_util_uint64_scale_int (distance / m_bpf, GST_SECOND, m_rate);

    adapter_avail = gst_adapter_available (adapter);

#ifdef SKIP_OLD_DATA
    GST_DEBUG("Skip the older data than reference pts");
    if (adapter_time < start_time) {
        diff = GST_CLOCK_DIFF (adapter_time, start_time);
        offset = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        offset = MIN(offset, adapter_avail);
        skip = 0;
    } else {
        diff = GST_CLOCK_DIFF (start_time, adapter_time);
        skip = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        skip = MIN(skip, m_PeriodSize);
        offset = 0;
    }
#else
	GST_DEBUG("Adjust the older data than reference pts");
    if (adapter_time > start_time) {
        diff = GST_CLOCK_DIFF (start_time, adapter_time);
        skip = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        skip = MIN(skip, m_PeriodSize);
        offset = 0;
    }
#endif

    copy_size = MIN (adapter_avail - offset, m_PeriodSize - skip);
    GST_INFO("start_time = %" G_GUINT64_FORMAT ",adapter_avail = %d, copy_size = %" G_GUINT64_FORMAT ", offset = %" G_GUINT64_FORMAT ", skip = %" G_GUINT64_FORMAT "\n", start_time,adapter_avail, copy_size, offset, skip);
    if (copy_size) {
        gst_adapter_copy (adapter, out_buf + skip, offset, copy_size);
        gst_adapter_flush (adapter, offset + copy_size);
    }

    if (skip_len != NULL) *skip_len = skip;
    if (read_len != NULL) *read_len = copy_size;

    return TRUE;
}

gboolean LgEcnrAdapter::SetAudioInfo(guint rate, guint bpf, guint channels, GstClockTime period) {
    m_rate = rate;
    m_bpf = bpf;
    m_channels = channels;
    m_PeriodTime = period;
    m_PeriodSize = gst_util_uint64_scale_int(m_PeriodTime*bpf,m_rate,GST_SECOND);
    GST_INFO("rate= %d,bpf= %d,channels= %d,period= %" G_GUINT64_FORMAT " ms (%d bytes)", rate,bpf,channels,period/1000000, m_PeriodSize);
    m_RecvInBufferSize = (32 * (m_rate / 1000) * m_bpf);
    m_MicInBufferSize = (32 * (m_rate / 1000) * m_bpf);

#ifndef DISABLE_LGECNR_ENGINE
    InitializeECNRLib();
#endif
    return TRUE;
}

void LgEcnrAdapter::SetObjects(GstBaseTransform *lgecnr, GstBaseTransform * lgecnrprobe) {
	m_lgecnr = lgecnr;
	m_lgecnrprobe = lgecnrprobe;
}

#ifndef DISABLE_LGECNR_ENGINE
gboolean LgEcnrAdapter::InitializeECNRLib() {
    const char *configpath = ECNR_CONFIG_PATH;
    int err;

    err = LGTSE_StartDynamicLoading(ECNR_LIB_FILE_FULLPATH);
    if (err != TSE_ERR_OK) {
        g_print ("Lib DynamicLoading Error: %d\n", err);
        goto IntializeEcnrLib_failed;
    }

    m_TseCtx = LGTSE_FilterCreate();
    if (m_TseCtx == NULL) {
        g_print ("LGTSE_FilterCreate Error\n");
        goto IntializeEcnrLib_failed;
    }
    err = LGTSE_SetTsfPath(m_TseCtx, configpath);
    if (err != TSE_ERR_OK) {
        g_print ("LGTSE_SetTsfPath Error: %d\n", err);
        goto IntializeEcnrLib_failed;
    }
    err = LGTSE_FilterInit(m_TseCtx, ECNR_TELEMATICS_MODE, m_rate, m_channels, m_PeriodSize/m_bpf);
    if (err != TSE_ERR_OK) {
        g_print ("LGTSE_FilterInit Error: %d\n", err);
        goto IntializeEcnrLib_failed;
    }

    return TRUE;

    IntializeEcnrLib_failed:
    if (m_TseCtx) {
        LGTSE_FilterDestroy(m_TseCtx);
        m_TseCtx = NULL;
    }
    return FALSE;
}
#endif

// Mic time is used as reference time
GstClockTime LgEcnrAdapter::RefToMicTime(GstClockTime ref_time) {
    return ref_time - m_MicAdjustTime;
}
GstClockTime LgEcnrAdapter::RefToRecvTime(GstClockTime ref_time) {
    return ref_time - m_RecvAdjustTime;
}
GstClockTime LgEcnrAdapter::MicToRefTime(GstClockTime mic_time) {
    return mic_time + m_MicAdjustTime;
}
GstClockTime LgEcnrAdapter::RecvToRefTime(GstClockTime recv_time) {
    return recv_time + m_RecvAdjustTime;
}
