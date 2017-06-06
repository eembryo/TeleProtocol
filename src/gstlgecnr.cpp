/* GStreamer
 * Copyright (C) 2017 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstlgecnr
 *
 * The lgecnr element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! lgecnr ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/audio/gstaudiofilter.h>
#include "gstlgecnr.h"
#include "lgecnradapter.h"
#include "gstlgecnrprobe.h"
#include <string.h>
#include <stdio.h>

GST_DEBUG_CATEGORY_STATIC (gst_lgecnr_debug_category);
#define GST_CAT_DEFAULT gst_lgecnr_debug_category

#define kECNRSamplePeriod   (16*GST_MSECOND)

/* prototypes */

static void gst_lgecnr_set_property(GObject *object,
                                    guint property_id, const GValue *value, GParamSpec *pspec);

static void gst_lgecnr_get_property(GObject *object,
                                    guint property_id, GValue *value, GParamSpec *pspec);

static void gst_lgecnr_dispose(GObject *object);

static void gst_lgecnr_finalize(GObject *object);

static gboolean gst_lgecnr_setup(GstAudioFilter *filter,
                                 const GstAudioInfo *info);

static GstFlowReturn gst_lgecnr_transform(GstBaseTransform *trans,
                                          GstBuffer *inbuf, GstBuffer *outbuf);

static GstBuffer *gst_lgecnr_take_buffer(GstLgecnr *lgecnr);

static gboolean gst_lgecnr_stop(GstBaseTransform *trans);

static gboolean gst_lgecnr_start(GstBaseTransform *trans);

enum {
    PROP_0,
    PROP_ECHO_CANCEL,
    PROP_PROBE_NAME,
    PROP_VOLUME_INFO,
};

/* pad templates */

static GstStaticPadTemplate gst_lgecnr_src_template =
        GST_STATIC_PAD_TEMPLATE ("src",
                                 GST_PAD_SRC,
                                 GST_PAD_ALWAYS,
                                 GST_STATIC_CAPS("audio/x-raw,format=S16LE,rate=8000,"
                                                         "channels=1,layout=interleaved")
        );

static GstStaticPadTemplate gst_lgecnr_sink_template =
        GST_STATIC_PAD_TEMPLATE ("sink",
                                 GST_PAD_SINK,
                                 GST_PAD_ALWAYS,
                                 GST_STATIC_CAPS("audio/x-raw,format=S16LE,rate=8000,"
                                                         "channels=1,layout=interleaved")
        );


/* class initialization */
static GstFlowReturn gst_lgecnr_prepare_output_buffer(GstBaseTransform *trans, GstBuffer *inBuf, GstBuffer **outBuf) {
    GstLgecnr *lgecnr = GST_LGECNR (trans);

    lgecnr->m_EcnrAdapter->PushMicBuffer(inBuf);
    lgecnr->m_EcnrAdapter->process();
    *outBuf = lgecnr->m_EcnrAdapter->PopMicBuffer();

    return GST_FLOW_OK;
}

G_DEFINE_TYPE_WITH_CODE (GstLgecnr, gst_lgecnr, GST_TYPE_AUDIO_FILTER,
                         GST_DEBUG_CATEGORY_INIT(gst_lgecnr_debug_category, "lgecnr", 0,
                                                 "debug category for lgecnr element"));

static void gst_lgecnr_class_init(GstLgecnrClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
    GstAudioFilterClass *audio_filter_class = GST_AUDIO_FILTER_CLASS (klass);

    /* Setting up pads and setting metadata should be moved to
    base_class_init if you intend to subclass this class. */

    gobject_class->set_property = GST_DEBUG_FUNCPTR(gst_lgecnr_set_property);
    gobject_class->get_property = GST_DEBUG_FUNCPTR(gst_lgecnr_get_property);
    gobject_class->dispose = GST_DEBUG_FUNCPTR(gst_lgecnr_dispose);
    gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_lgecnr_finalize);

    base_transform_class->passthrough_on_same_caps = TRUE;
    //base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_lgecnr_transform);
    base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_lgecnr_prepare_output_buffer);
    base_transform_class->start = GST_DEBUG_FUNCPTR (gst_lgecnr_start);
    base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_lgecnr_stop);

    audio_filter_class->setup = GST_DEBUG_FUNCPTR (gst_lgecnr_setup);
    gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
                                       gst_static_pad_template_get(&gst_lgecnr_src_template));
    gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
                                       gst_static_pad_template_get(&gst_lgecnr_sink_template));

    gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass),
                                          "LG ECNR Plugin", "ECNR processing",
                                          "A plugin stores streaming data from audio devices into buffer. and collect data from LGECNR probe plugin and does ecnr processing.",
                                          "Hyobeom Lee <hyobeom1.lee@lge.com> & Dhilipkumar R <dhilipkumar.raman@lge.com>");

    g_object_class_install_property(gobject_class,
                                    PROP_PROBE_NAME,
                                    g_param_spec_string("probe", "Echo Probe",
                                                        "The name of the lgecnrprobe element that record the audio being "
                                                                "played through loud speakers. Must be set before PAUSED state.",
                                                        "lgecnrprobe0",
                                                        (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                                                                       G_PARAM_CONSTRUCT)));

    g_object_class_install_property(gobject_class,
                                    PROP_ECHO_CANCEL,
                                    g_param_spec_boolean("echoCancel", "Echo Cancel",
                                                         "Enable or disable echo canceller, note that it will be disabled if "
                                                                 "no lgecnrprobe has been found", TRUE,
                                                         (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                                                                        G_PARAM_CONSTRUCT)));

    g_object_class_install_property(gobject_class,
                                    PROP_VOLUME_INFO,
                                    g_param_spec_uint("volumeInfo", "volume info level",
                                                      "volume info level to be used for ecnr processing",
                                                      0,
                                                      100,
                                                      15,
                                                      (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                                                                     G_PARAM_CONSTRUCT)));
}

static void gst_lgecnr_init(GstLgecnr *lgecnr) {
    GST_DEBUG_OBJECT (lgecnr, "init");
    lgecnr->m_EcnrAdapter = new LgEcnrAdapter();
    gst_audio_info_init(&lgecnr->info);
}

void gst_lgecnr_set_property(GObject *object, guint property_id,
                        const GValue *value, GParamSpec *pspec) {
    GstLgecnr *lgecnr = GST_LGECNR (object);

    GST_DEBUG_OBJECT (lgecnr, "set_property");

    switch (property_id) {
        case PROP_ECHO_CANCEL:
            lgecnr->echoCancel = g_value_get_boolean(value);
            break;
        case PROP_PROBE_NAME:
            g_free(lgecnr->probe_name);
            lgecnr->probe_name = g_value_dup_string(value);
            break;
        case PROP_VOLUME_INFO:
            lgecnr->volumeInfo = g_value_get_uint(value);
            GST_INFO_OBJECT (lgecnr, "volume level set =%d ", lgecnr->volumeInfo);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void
gst_lgecnr_get_property(GObject *object, guint property_id,
                        GValue *value, GParamSpec *pspec) {
    GstLgecnr *lgecnr = GST_LGECNR (object);

    GST_DEBUG_OBJECT (lgecnr, "get_property");
    switch (property_id) {
        case PROP_ECHO_CANCEL:
            g_value_set_boolean(value, lgecnr->echoCancel);
            break;
        case PROP_PROBE_NAME:
            g_value_set_string(value, lgecnr->probe_name);
            break;
        case PROP_VOLUME_INFO:
            g_value_set_uint(value, lgecnr->volumeInfo);
            GST_INFO_OBJECT (lgecnr, "volume level set =%d", lgecnr->volumeInfo);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void
gst_lgecnr_dispose(GObject *object) {
    GstLgecnr *lgecnr = GST_LGECNR (object);

    GST_DEBUG_OBJECT (lgecnr, "dispose");

    /* clean up as possible.  may be called multiple times */

    G_OBJECT_CLASS (gst_lgecnr_parent_class)->dispose(object);
}

void gst_lgecnr_finalize(GObject *object) {
    GstLgecnr *lgecnr = GST_LGECNR (object);

    GST_DEBUG_OBJECT (lgecnr, "finalize");

    /* clean up object here */
    gst_object_unref(lgecnr->adapter);

    if (lgecnr->m_EcnrAdapter) {
        delete lgecnr->m_EcnrAdapter;
    }

    g_free(lgecnr->probe_name);
    G_OBJECT_CLASS (gst_lgecnr_parent_class)->finalize(object);
}

static gboolean gst_lgecnr_setup(GstAudioFilter *filter, const GstAudioInfo *info) {
    GstLgecnr *lgecnr = GST_LGECNR (filter);

    GST_DEBUG_OBJECT (lgecnr, "setting format to %s with %i Hz and %i channels",
                      info->finfo->description, info->rate, info->channels);

    GST_OBJECT_LOCK (lgecnr);

    lgecnr->m_EcnrAdapter->SetAudioInfo(info->rate,info->bpf,info->channels,kECNRSamplePeriod);

    // LGECNR library takes 16ms buffer.
#if 0
    lgecnr->period_size = info->bpf * info->rate * 0.016;
    if ((kMaxDataSizeSamples * 2) < lgecnr->period_size)
        goto period_too_big;

    if (lgecnr->probe) {
        GST_LGECNR_PROBE_LOCK(lgecnr->probe);

        if (lgecnr->probe->info.rate != 0) {
            if (lgecnr->probe->info.rate != info->rate)
                goto probe_has_wrong_rate;

            probe_info = lgecnr->probe->info;
        }

        GST_LGECNR_PROBE_UNLOCK (lgecnr->probe);
    }
#endif

    GST_OBJECT_UNLOCK (lgecnr);

    return TRUE;
}

#if 0
/* transform */
static GstFlowReturn gst_lgecnr_transform(GstBaseTransform *trans, GstBuffer *inBuf,
                     GstBuffer *outBuf) {
    GstLgecnr *lgecnr = GST_LGECNR (trans);
    GstClockTime timestamp;
    guint64 distance;
    guint n_periods;
    GstLgecnrProbe *ecnr_probe = NULL;
    AudioFrame frame = {{0}, 0, 0, 0, 0, 0};
    GstFlowReturn ret = GST_FLOW_OK;
    int err;
    GstBuffer *spkOutBuf;
    GST_DEBUG_OBJECT (lgecnr, "transform: inbuf = 0x%p, outbuf = 0x%p", inBuf, outBuf);

    n_periods = gst_adapter_available(lgecnr->adapter) / lgecnr->period_size;

    {
        GstMapInfo mapinfo;
        gconstpointer InputData;
        gst_buffer_map(outBuf, &mapinfo, GST_MAP_WRITE);
        if (n_periods * lgecnr->period_size > mapinfo.maxsize) {
            GST_ERROR_OBJECT (lgecnr, "generated buffer size (%d) is not enough for (%d).", (int) (mapinfo.maxsize),
                              n_periods * lgecnr->period_size);
            return GST_FLOW_ERROR;
        }

#ifdef ECNR_ENABLE
        timestamp = gst_adapter_prev_pts(lgecnr->adapter, &distance);
        if (timestamp == GST_CLOCK_TIME_NONE && distance == 0) {
            GST_BUFFER_FLAG_SET (outBuf, GST_BUFFER_FLAG_DISCONT);
        } else
            GST_BUFFER_FLAG_UNSET (outBuf, GST_BUFFER_FLAG_DISCONT);
        timestamp += gst_util_uint64_scale_int(distance / lgecnr->info.bpf, GST_SECOND, lgecnr->info.rate);
        GST_BUFFER_PTS (outBuf) = timestamp;
        GST_BUFFER_DURATION (outBuf) = 16 * n_periods * GST_MSECOND;

        InputData = gst_adapter_map(lgecnr->adapter, n_periods * lgecnr->period_size);
        GST_OBJECT_LOCK (lgecnr);
        if (lgecnr->echoCancel)    //enable disable echo cancellation
            ecnr_probe = GST_LGECNR_PROBE (g_object_ref(lgecnr->probe));
        GST_OBJECT_UNLOCK (lgecnr);

        if (lgecnr->echoCancel)    //enable disable echo cancellation
            ecnr_probe = GST_LGECNR_PROBE (g_object_ref(lgecnr->probe));

        if (!ecnr_probe)
            return GST_FLOW_OK;

        char outpuAudioBuffer[lgecnr->period_size];

        GST_DEBUG_OBJECT (lgecnr, "looping = %d", n_periods);
        GstMapInfo outMapInfo;
        for (int i = 0; i < n_periods; i++) {
            gst_lgecnr_echo_probe_read(ecnr_probe, timestamp + 16 * GST_MSECOND * i, (gpointer) &frame);
            //gst_lgecnr_echo_probe_read (ecnr_probe, timestamp, (gpointer) &frame);

            LGTSE_FilterProcess(lgecnr->mTseCtx, (char *) InputData, (char *) mapinfo.data, (char *) frame.data_,
                                (char *) outpuAudioBuffer, &lgecnr->volumeInfo);
            InputData += lgecnr->period_size;
            mapinfo.data += lgecnr->period_size;
            GST_INFO_OBJECT (lgecnr, "Iteration =%d  frame.size =%lu", i, frame.size);
            if (frame.size) {
                spkOutBuf = gst_buffer_new_allocate(NULL, frame.size, NULL);
                gst_buffer_map(spkOutBuf, &outMapInfo, GST_MAP_WRITE);
                memcpy(outMapInfo.data, outpuAudioBuffer, frame.size);
                gst_buffer_unmap(spkOutBuf, &outMapInfo);
                GST_BUFFER_PTS (spkOutBuf) = frame.timestamp;
                GST_LGECNR_PROBE_LOCK (ecnr_probe);
                gst_adapter_push(ecnr_probe->spkAdapter, spkOutBuf);
                GST_LGECNR_PROBE_UNLOCK (ecnr_probe);
            }

        }
        GST_INFO_OBJECT (lgecnr, "audioEcnr prcoessing successful ");
        gst_buffer_unmap(outBuf, &mapinfo);

        gst_adapter_flush(lgecnr->adapter, n_periods * lgecnr->period_size);
        done:
        gst_object_unref(ecnr_probe);
#else
        gst_adapter_copy (lgecnr->adapter, mapinfo.data, 0, n_periods * lgecnr->period_size);
        gst_buffer_unmap (outBuf, &mapinfo);
        gst_adapter_flush (lgecnr->adapter, n_periods * lgecnr->period_size);

        timestamp = gst_adapter_prev_pts (lgecnr->adapter, &distance);
        if (timestamp == GST_CLOCK_TIME_NONE && distance == 0) {
            GST_BUFFER_FLAG_SET (outBuf, GST_BUFFER_FLAG_DISCONT);
        } else
            GST_BUFFER_FLAG_UNSET (outBuf, GST_BUFFER_FLAG_DISCONT);
        timestamp += gst_util_uint64_scale_int (distance / lgecnr->info.bpf, GST_SECOND, lgecnr->info.rate);
        GST_BUFFER_PTS (outBuf) = timestamp;
        GST_BUFFER_DURATION (outBuf) = 16 * n_periods * GST_MSECOND;
#endif

    }

    return ret;
}
#endif

static gboolean
gst_lgecnr_stop(GstBaseTransform *trans) {
    GstLgecnr *lgecnr = GST_LGECNR (trans);

    GST_OBJECT_LOCK (lgecnr);

    if (lgecnr->probe) {
        gst_lgecnr_release_echo_probe(lgecnr->probe);
        lgecnr->probe = NULL;
    }

    GST_OBJECT_UNLOCK (lgecnr);

    return TRUE;
}

static gboolean
gst_lgecnr_start(GstBaseTransform *trans) {
    GstLgecnr *lgecnr = GST_LGECNR (trans);
    GST_OBJECT_LOCK (lgecnr);

    if (lgecnr->echoCancel) // enable disable echo cancelling
        lgecnr->probe = gst_lgecnr_acquire_echo_probe(lgecnr->probe_name);

    if (lgecnr->probe == NULL) {
        GST_OBJECT_UNLOCK (lgecnr);
        GST_ELEMENT_ERROR (lgecnr, RESOURCE, NOT_FOUND,
                           ("No echo probe with name %s found.", lgecnr->probe_name), (NULL));
        return FALSE;
    }

    GST_OBJECT_UNLOCK (lgecnr);

    return TRUE;
}

static gboolean
plugin_init(GstPlugin *plugin) {
    if (!gst_element_register(plugin, "lgecnrprobe", GST_RANK_NONE, GST_TYPE_LGECNR_PROBE)) {
        return FALSE;
    }

    if (!gst_element_register(plugin, "lgecnr", GST_RANK_NONE, GST_TYPE_LGECNR)) {
        return FALSE;
    }

    return TRUE;
}

/* FIXME: these are normally defined by the GStreamer build system.
	If you are creating an element to be included in gst-plugins-*,
	remove these, as they're always defined.  Otherwise, edit as
	appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   lgecnr,
                   "FIXME plugin description",
                   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

