//
// Created by root on 5/11/17.
//

#include <string.h>
#include "lgecnradapter.h"

//LGECNR_PERIOD = 16 * GST_MSECOND
//period_size = 8000Hz * 0.016s * 2Bytes
#define LGECNR_PERIOD_SIZE 256

#ifndef DISABLE_LGECNR_ENGINE
#include <ringbuffer.h>
#include <tseFilter.h>
#include <tseFilter_priv.h>
#endif

GST_DEBUG_CATEGORY_STATIC(gst_lgecnradapter_debug_category);
#define GST_CAT_DEFAULT gst_lgecnradapter_debug_category

static const gchar ZERO_FRAME[10] = {0x00};

static gboolean CopyBufferFromAdapter (GstBuffer *target, GstAdapter *src, gsize size) {
    GstMapInfo mapinfo;

    if (!gst_buffer_map (target, &mapinfo, GST_MAP_WRITE)) {
        return FALSE;
    }

    gst_adapter_copy (src, mapinfo.data, 0, size);
    gst_buffer_unmap (target, &mapinfo);
    return TRUE;

    _CopyBufferFromAdapter_failed:
    gst_buffer_unmap (target, &mapinfo);
    return FALSE;
}

LgEcnrAdapter::LgEcnrAdapter() :
        m_PrevRefTime(GST_CLOCK_TIME_NONE), m_RecvStreamLatency(GST_CLOCK_TIME_NONE), m_RecvStreamDelay(GST_CLOCK_TIME_NONE),
        m_PeriodSize(0),m_PeriodTime(GST_CLOCK_TIME_NONE),
        m_rate(0),m_bpf(0),m_channels(0),
        m_IsFirstMicBuffer(1), m_IsFirstRecvBuffer(1) {

    GST_DEBUG_CATEGORY_INIT(gst_lgecnradapter_debug_category, "LgEcnrAdapter", 0, "debug category for lgecnr adapter class");

    g_mutex_init(&m_MicInLock);
    g_mutex_init(&m_MicOutLock);
    g_mutex_init(&m_RecvInLock);
    g_mutex_init(&m_RecvOutLock);

    m_MicInAdapter = gst_adapter_new();
    m_RecvInAdapter = gst_adapter_new();
    m_MicOutAdapter = gst_adapter_new();
    m_RecvOutAdapter = gst_adapter_new();
}

LgEcnrAdapter::~LgEcnrAdapter() {
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
}

GstBuffer* LgEcnrAdapter::PopMicBuffer() {
    GstBuffer *out = NULL;
    gsize out_buffer_size = 0;

    if (gst_adapter_available (m_MicOutAdapter) < LGECNR_PERIOD_SIZE) {
        // Not enough data in MicOutAdapter
        return NULL;
    }

    out_buffer_size = gst_adapter_available(m_MicOutAdapter) % LGECNR_PERIOD_SIZE;
    out = gst_buffer_new_allocate (NULL, out_buffer_size, NULL);
    if (!out) {
        // memory couldnot be allocated
        goto _PopMicBuffer_failed;
    }

    MICOUT_LOCK(this);
    if (!CopyBufferFromAdapter(out, m_MicOutAdapter, out_buffer_size)) {
        MICOUT_UNLOCK(this);
        goto _PopMicBuffer_failed;
    }
    MICOUT_UNLOCK(this);

    return out;

    _PopMicBuffer_failed:
    if (out) gst_buffer_unref(out);
    return NULL;
}

GstBuffer* LgEcnrAdapter::PopRecvBuffer() {
    GstBuffer *out = NULL;
    gsize out_buffer_size = 0;

    if (gst_adapter_available (m_RecvOutAdapter) < LGECNR_PERIOD_SIZE) {
        // Not enough data in RecvOutAdapter
        return NULL;
    }

    out_buffer_size = gst_adapter_available(m_RecvOutAdapter) % LGECNR_PERIOD_SIZE;
    out = gst_buffer_new_allocate (NULL, out_buffer_size, NULL);
    if (!out) {
        // memory couldnot be allocated
        goto _PopRecvBuffer_failed;
    }

    RECVOUT_LOCK(this);
    if (!CopyBufferFromAdapter(out, m_RecvOutAdapter, out_buffer_size)) {
        RECVOUT_UNLOCK(this);
        goto _PopRecvBuffer_failed;
    }
    RECVOUT_UNLOCK(this);

    return out;

    _PopRecvBuffer_failed:
    if (out) gst_buffer_unref(out);
    return NULL;
}

gboolean LgEcnrAdapter::PushMicBuffer(GstBuffer *buffer,guint32 volume) {
    guint offset;

    buffer = gst_buffer_make_writable(buffer);

    if (GST_BUFFER_IS_DISCONT(buffer)) {
        MICIN_LOCK();
        gst_adapter_clear(m_MicInAdapter);
        MICIN_UNLOCK();
    }

    if (g_atomic_int_compare_and_exchange(m_IsFirstMicBuffer, 1, 0)) {
        for (offset = 0; offset < gst_buffer_get_size(buffer); offset += m_bpf) {
            if (gst_buffer_memcmp(buffer, offset, ZERO_FRAME, m_bpf)) break;
        }

        gst_buffer_resize(buffer, offset, -1);
        GstClockTime initial_delay = gst_util_uint64_scale_int(offset / m_bpf, GST_SECOND, m_rate);
        GST_BUFFER_PTS(buffer) += initial_delay;
        GST_BUFFER_DURATION(buffer) -= initial_delay;
    }

    MICIN_LOCK();
    gst_adapter_push (m_MicInAdapter, buffer);
    MICIN_UNLOCK();

    return TRUE;
}

gboolean LgEcnrAdapter::PushRecvBuffer(GstBuffer *buffer) {
    guint offset;

    buffer = gst_buffer_make_writable(buffer);

    if (GST_BUFFER_IS_DISCONT(buffer)) {
        RECVIN_LOCK();
        gst_adapter_clear(m_RecvInAdapter);
        RECVIN_UNLOCK();
    }

    if (g_atomic_int_compare_and_exchange(m_IsFirstRecvBuffer, 1, 0)) {
        for (offset = 0; offset < gst_buffer_get_size(buffer); offset += m_bpf) {
            if (gst_buffer_memcmp(buffer, offset, ZERO_FRAME, m_bpf)) break;
        }

        gst_buffer_resize(buffer, offset, -1);
        GstClockTime initial_delay = gst_util_uint64_scale_int(offset / m_bpf, GST_SECOND, m_rate);
        GST_BUFFER_PTS(buffer) += initial_delay;
        GST_BUFFER_DURATION(buffer) -= initial_delay;
    }

    RECVIN_LOCK();
    gst_adapter_push (m_RecvInAdapter, buffer);
    RECVIN_UNLOCK();

    return TRUE;
}

static gssize adapter_available (GstAdapter *adapter, GstClockTime from) {
    GstClockTime adapter_pts;
    guint64 distance;
    guint64 diff_size = 0;
    gssize avail;

    adapter_pts = gst_adapter_prev_pts (adapter, &distance);
    adapter_pts += gst_util_uint64_scale_int(distance / m_bpf, GST_SECOND, m_rate);

    if (adapter_pts < from) {
        GstClockTimeDiff diff = from - adapter_pts;
        diff_size = diff * m_bpf;
    }
    avail = gst_adapter_available - diff_size;
    return avail > 0 ? avail : 0;
}
static gboolean fill_memory_from_adapter (GstClockTime time, GstAdapter *adapter, gchar *memory, guint *skip, gssize *size) {
    GstClockTime adapter_pts;

    adapter_pts =
}

void LgEcnrAdapter::process() {
    GstClockTime ref_time = GST_CLOCK_TIME_NONE;
    gchar micin_buf[m_PeriodSize] = {0};
    gchar recvin_buf[m_PeriodSize] = {0};
    GstBuffer *micout_buf;
    GstMapInfo  micout_minfo;
    GstBuffer *recvout_buf;
    GstMapInfo  recvout_minfo;

    if (!m_MicInAdapter || !m_RecvInAdapter) {
        return;
    }

    MICIN_LOCK();
    RECVIN_LOCK();

    if (!GST_CLOCK_TIME_IS_VALID(m_PrevRefTime)) {
        GstClockTime micin_adapter_pts;
        GstClockTime recvin_adapater_pts;
        GstClockTime ref_time1, ref_time2;
        guint64 distance;

        micin_adapter_pts = gst_adapter_prev_pts (m_MicInAdapter, &distance);
        micin_adapter_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);
        recvin_adapater_pts = gst_adapter_prev_pts (m_RecvInAdapter, &distance);
        recvin_adapter_pts += gst_util_uint64_scale_int(distance/m_bpf, GST_SECOND, m_rate);

        ref_time1 = MicToRefTime(micin_adapter_pts);
        ref_time2 = RecvToRefTime(recvin_adapater_pts);

        if (GST_CLOCK_TIME_IS_VALID(ref_time1) && GST_CLOCK_TIME_IS_VALID(ref_time2)) {
            ref_time = MAX(ref_time1, ref_time2);

            guint64 micin_avail;
            guint64 recvin_avail;
            FlushAdapterUntil(m_MicInAdapter, MicToRefTime(ref_time), &micin_avail);
            FlushAdapterUntil(m_RecvInAdapter, RecvToRefTime(ref_time), &recvin_avail);
            if (micin_avail < m_PeriodSize || recvin_avail < m_PeriodSize)  //not enouch data in MicIn or RecvIn adapter
                goto _process_failed;

        } else {
            ref_time = GST_CLOCK_TIME_NONE;
        }
    }
    else {
        ref_time = m_PrevRefTime + m_PeriodTime;
    }

    if (!GST_CLOCK_TIME_IS_VALID(ref_time)) goto _process_failed;

    m_PrevRefTime = ref_time;

    ReadFromAdapter(m_MicInAdapter, RefToMicTime(ref_time), NULL, NULL);
    ReadFromAdapter(m_RecvInAdapter, RefToMicTime(ref_time), NULL, NULL);

    MICIN_UNLOCK();
    RECVIN_UNLOCK();

    // pass read data to LG ECNR ENGINE
    micout_buf = gst_buffer_new_allocate(NULL, m_PeriodSize, NULL);
    recvout_buf = gst_buffer_new_allocate(NULL, m_PeriodSize, NULL);
    gst_buffer_map(micout_buf, &micout_minfo, GST_MAP_WRITE);
    gst_buffer_map(recvout_buf, &recvout_minfo, GST_MAP_WRITE);

#ifdef DISABLE_LGECNR_ENGINE
    memcpy(micout_minfo.data, micin_buf, sizeof(micout_buf));
    memcpy(recvout_minfo.data, recvin_buf, sizeof(recvout_buf));
#else
    LGTSE_FilterProcess(lgecnr->mTseCtx, (char *) micin_buf, (char *) micout_minfo.data, (char *) recvout_buf,
                                (char *) recvout_minfo.data, NULL);
#endif
    gst_buffer_unmap(micout_buf, &micout_minfo);
    gst_buffer_unmap(recvout_buf, &recvout_minfo);

    GST_BUFFER_PTS (micout_buf) = RefToMicTime(ref_time);
    GST_BUFFER_DURATION (micout_buf) = m_PeriodSize;
    GST_BUFFER_PTS (recvout_buf) = RefToRecvTime(ref_time);
    GST_BUFFER_DURATION (recvout_buf) = m_PeriodSize;

    // push buffers to output adapter
    MICOUT_LOCK();
    RECVOUT_LOCK();
    gst_adapter_push(m_MicOutAdapter, micout_buf);
    gst_adapter_push(m_RecvOutAdapter, recvout_buf);
    RECVOUT_UNLOCK();
    MICOUT_UNLOCK();

    _process_failed:
    MICIN_UNLOCK();
    RECVIN_UNLOCK();
}

void LgEcnrAdapter::FlushAdapterUntil(GstAdapter *adapter, GstClockTime time, guint64 *remains) {
    GstClockTime adapter_time;
    GstClockTimeDiff diff;
    gsize adapter_avail = 0;
    guint64 discard_size = 0;

    adapter_time = gst_adapter_prev_pts(m_RecvInAdapter, &distance);
    adapter_time += gst_util_uint64_scale_int (distance / m_bpf, GST_SECOND, m_rate);

    adapter_avail = gst_adapter_available (adapter);

    if (adapter_time < time) {
        diff = GST_CLOCK_DIFF (adapter_time, time);
        discard_size = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        discard_size = MIN(discard_size, adapter_avail);
    }
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
    guint64 skip;
    guint64 offset;

    adapter_time = gst_adapter_prev_pts(m_RecvInAdapter, &distance);
    adapter_time += gst_util_uint64_scale_int (distance / m_bpf, GST_SECOND, m_rate);

    adapter_avail = gst_adapter_available (m_RecvInAdapter);

    if (adapter_time < ref_time) {
        diff = GST_CLOCK_DIFF (adapter_time, start_time);
        offset = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        offset = MIN(offset, adapter_avail);
    } else {
        diff = GST_CLOCK_DIFF (start_time, adapter_time);
        skip = gst_util_uint64_scale_int(diff*m_bpf, m_rate, GST_SECOND);
        skip = MIN(skip, m_PeriodSize);
        offset = 0;
    }

    copy_size = MIN (adapter_avail - offset, m_PeriodSize - skip);
    if (copy_size) {
        gst_adapter_copy (m_RecvInAdapter, buf + skip, offset, copy_size);
        gst_adapter_flush (m_RecvInAdapter, offset + size);
    }

    if (skip_len != NULL) *skip_len = skip;
    if (read_len != NULL) *read_len = copy_size;

    return TRUE;
}
/*
 * buf: should have enough memory to keep 'm_PeriodSize' bytes of data
 * ref_time: extract data from RecvInAdapter at 'ref_time'. All data before 'ref_time' will be discarded.
 * skip_len: skipped length in buf
 * read_len: read length in buf
 */
gboolean LgEcnrAdapter::ReadFromRecvInAdapter(GstClockTime ref_time, gchar *buf, guint64 *skip_len, guint64 *read_len) {
    GstClockTime recv_time = RefToRecvTime(ref_time);
    return ReadFromAdapter(m_RecvInAdapter, recv_time, buf, skip_len, read_len);
}
gboolean LgEcnrAdapter::ReadFromMicInAdapter(GstClockTime ref_time, gchar *buf, guint64 *skip_len, guint64 *read_len) {
    GstClockTime mic_time = RefToMicTime(ref_time);
    return ReadFromAdapter(m_RecvInAdapter, mic_time, buf, skip_len, read_len);
}

void LgEcnrAdapter::SetAudioInfo(guint rate, guint bpf, guint channels, GstClockTime period) {
    m_rate = rate;
    m_bpf = bpf;
    m_channels = channels;
    m_period = period;
    m_PeriodSize = gst_util_uint64_scale_int(m_period*bpf,m_rate,GST_SECOND);
    GST_INFO("%s: rate= %d,bpf= %d,channels= %d,period= %d ms (%d bytes)", __func__,rate,bpf,channels,period/1000000, m_PeriodSize);
}

// Mic time is used as reference time
GstClockTime LgEcnrAdapter::RefToMicTime(GstClockTime ref_time) {
    return ref_time;
}
GstClockTime LgEcnrAdapter::RefToRecvTime(GstClockTime ref_time) {
    return ref_time - m_RecvStreamLatency - m_RecvStreamDelay;
}
GstClockTime LgEcnrAdapter::MicToRefTime(GstClockTime mic_time) {
    return mic_time;
}
GstClockTime LgEcnrAdapter::RecvToRefTime(GstClockTime recv_time) {
    return recv_time + m_RecvStreamLatency + m_RecvStreamDelay;
}