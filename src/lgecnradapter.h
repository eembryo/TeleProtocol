//
// Created by root on 5/11/17.
//

#ifndef GST_PLUGINS_LGECNRADAPTER_H
#define GST_PLUGINS_LGECNRADAPTER_H

#include <gst/base/gstadapter.h>
#include <string>
#include <glib.h>
#include <gst/base/gstbasetransform.h>

/****************************************************************************
 *                             +------------------------+
 * <MIC Stream>     MIC_IN --- |                        | --- MIC_OUT
 *                             |                        |
 *                             | LGECNR_FilterProcess   |
 *                             |                        |
 * <RECV Stream>   RECV_IN --- |                        | --- RECV_OUT
 *                             +------------------------+
 ****************************************************************************
 */
class LgEcnrAdapter {
public:
    LgEcnrAdapter ();
    virtual ~LgEcnrAdapter();

    gboolean        PushMicBuffer (GstBuffer *buffer);
    gboolean        PushRecvBuffer (GstBuffer *buffer);

    void            process ();

    void            StartTimer();
    void 			StopTimer();

    gboolean        SetAudioInfo(guint rate, guint bpf, guint channels, GstClockTime period);
    void 			SetObjects(GstBaseTransform *lgecnr, GstBaseTransform * lgecnrprobe);
    void            SetVolume(guint32 volume) {m_volume = volume;}
    void			SetRecvInBuffer(guint64 delay_ms) {m_RecvInBufferSize = (delay_ms * (m_rate / 1000) * m_bpf);}
    void			SetMicInBuffer(guint64 delay_ms)  {m_MicInBufferSize = (delay_ms * (m_rate / 1000) * m_bpf);}
    GstBaseTransform *m_lgecnr;
    GstBaseTransform *m_lgecnrprobe;
#ifndef DISABLE_LGECNR_ENGINE
    gboolean        InitializeECNRLib();
#endif
    GstClockTime    m_RefTime;              //absolute time
protected:
    virtual GstClockTime    RefToMicTime(GstClockTime ref_time);
    virtual GstClockTime    RefToRecvTime(GstClockTime ref_time);
    virtual GstClockTime    MicToRefTime(GstClockTime mic_time);
    virtual GstClockTime    RecvToRefTime(GstClockTime recv_time);
private:
    void            FlushAdapterUntil(GstAdapter *adapter, GstClockTime time, guint64 *remains);
    gboolean        ReadFromAdapter(GstAdapter *adapter, GstClockTime time, gchar *out_buf, guint64 *skip_len, guint64 *read_len);

    inline void     MICIN_LOCK () {g_mutex_lock (&m_MicInLock);}
    inline void     MICIN_UNLOCK () {g_mutex_unlock (&m_MicInLock);}
    inline void     MICOUT_LOCK () {g_mutex_lock (&m_MicOutLock);}
    inline void     MICOUT_UNLOCK () {g_mutex_unlock (&m_MicOutLock);}
    inline void     RECVIN_LOCK () {g_mutex_lock (&m_RecvInLock);}
    inline void     RECVIN_UNLOCK () {g_mutex_unlock (&m_RecvInLock);}
    inline void     RECVOUT_LOCK () {g_mutex_lock (&m_RecvOutLock);}
    inline void     RECVOUT_UNLOCK () {g_mutex_unlock (&m_RecvOutLock);}

    GMutex          m_MicInLock, m_MicOutLock, m_RecvInLock, m_RecvOutLock;
    GstAdapter      *m_MicInAdapter;
    GstAdapter      *m_MicOutAdapter;
    GstAdapter      *m_RecvInAdapter;
    GstAdapter      *m_RecvOutAdapter;
    gint            m_IsFirstRecvBuffer;
    gint            m_IsFirstMicBuffer;
    GstClockTime    m_RecvStreamLatency;
    GstClockTime    m_RecvStreamDelay;
    GstClockTime    m_MicAdjustTime;
    GstClockTime    m_RecvAdjustTime;

    // stream info
    GstClockTime    m_PeriodTime;
    guint           m_PeriodSize;   // m_PeriodSize = m_PeriodTime * (m_rate/GST_SECOND) * m_bpf
    guint           m_rate;
    guint           m_channels;
    guint           m_bpf;
    guint32         m_volume;

#ifndef DISABLE_LGECNR_ENGINE
    gpointer        m_TseCtx;
#endif
    gboolean        m_TimerRunning;
    GstClock        *m_clock;
    GstClockID      m_ClockId;

    guint64			m_MicInBufferSize;
    guint64			m_RecvInBufferSize;
};


#endif //GST_PLUGINS_LGECNRADAPTER_H
