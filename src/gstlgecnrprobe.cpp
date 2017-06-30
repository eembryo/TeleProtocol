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
 * SECTION:element-gstlgecnrprobe
 *
 * The lgecnrprobe element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! lgecnrprobe ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include "gstlgecnrprobe.h"

#include "commontypes.h"
#include <string.h>

GST_DEBUG_CATEGORY_STATIC (gst_lgecnr_probe_debug_category);
#define GST_CAT_DEFAULT gst_lgecnr_probe_debug_category

#define MAX_ADAPTER_SIZE (1*1024*1024)

G_LOCK_DEFINE_STATIC (gst_lgecnr_probes);
static GList *gst_lgecnr_probes = NULL;

/* prototypes */
static void gst_lgecnr_probe_set_property (GObject * object,
                                           guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_lgecnr_probe_get_property (GObject * object,
                                           guint property_id, GValue * value, GParamSpec * pspec);
static void gst_lgecnr_probe_dispose (GObject * object);
static void gst_lgecnr_probe_finalize (GObject * object);

static gboolean gst_lgecnr_probe_setup (GstAudioFilter * filter, const GstAudioInfo * info);
static gboolean gst_lgecnr_echo_probe_src_event (GstBaseTransform * trans, GstEvent * event);
static gboolean gst_lgecnr_echo_probe_stop (GstBaseTransform * trans);

enum
{
    PROP_0,
    PROP_TIMEADJUST,
	PROP_LATENCY_ADJUST
};

/* pad templates */
static GstStaticPadTemplate gst_lgecnr_probe_src_template =
        GST_STATIC_PAD_TEMPLATE ("src",
                                 GST_PAD_SRC,
                                 GST_PAD_ALWAYS,
                                 GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=8000,"
                                                          "channels=1,layout=interleaved")
        );

static GstStaticPadTemplate gst_lgecnr_probe_sink_template =
        GST_STATIC_PAD_TEMPLATE ("sink",
                                 GST_PAD_SINK,
                                 GST_PAD_ALWAYS,
                                 GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=8000,"
                                                          "channels=1,layout=interleaved")
        );

/* class initialization */
G_DEFINE_TYPE_WITH_CODE (GstLgecnrProbe, gst_lgecnr_probe, GST_TYPE_AUDIO_FILTER,
                         GST_DEBUG_CATEGORY_INIT (gst_lgecnr_probe_debug_category, "lgecnrprobe", 0,
                                                  "debug category for lgecnrprobe element"));


static GstFlowReturn gst_lgecnr_probe_prepare_output_buffer (GstBaseTransform *trans, GstBuffer *inBuf, GstBuffer **outBuf) {
    GstLgecnrProbe *probe = GST_LGECNR_PROBE (trans);

	GST_DEBUG_OBJECT (probe, "input buffer size: %" G_GSIZE_FORMAT ", pts=%" G_GUINT64_FORMAT , gst_buffer_get_size (inBuf), GST_BUFFER_PTS(inBuf));


    if (probe->adapter) {
        probe->adapter->PushRecvBuffer(inBuf);
    }
	*outBuf = NULL;

	GST_INFO_OBJECT (probe, "probe RETURING NULL BUFFER");

    return GST_FLOW_OK;
}

static gboolean gst_lgecnr_probe_query(GstBaseTransform * trans, GstPadDirection direction, GstQuery * query) {
    GstLgecnrProbe *probe;

    probe = GST_LGECNR_PROBE(trans);

    if (direction == GST_PAD_SRC) {
        switch (GST_QUERY_TYPE(query)) {
            case GST_QUERY_LATENCY:
                GstPad *peer;
                if ((peer = gst_pad_get_peer(GST_BASE_TRANSFORM_SINK_PAD (trans)))) {
                    if ((gst_pad_query(peer, query))) {
                        GstClockTime min, max;
                        gboolean live;

                        gst_query_parse_latency(query, &live, &min, &max);
                        GST_DEBUG_OBJECT (probe, "Peer latency: min %" GST_TIME_FORMAT " max %" GST_TIME_FORMAT, GST_TIME_ARGS(min), GST_TIME_ARGS(max));
                        /* add our own latency */
                        GST_DEBUG_OBJECT (probe, "Our latency: %" GST_TIME_FORMAT, GST_TIME_ARGS(50*GST_MSECOND));

                        min += probe->latencyadjust*GST_MSECOND;
                        if (max != GST_CLOCK_TIME_NONE)
                            max += probe->latencyadjust*GST_MSECOND;
                        GST_DEBUG_OBJECT (probe, "Calculated total latency : min %" GST_TIME_FORMAT " max %" GST_TIME_FORMAT, GST_TIME_ARGS (min), GST_TIME_ARGS (max));
                        gst_query_set_latency (query, live, min, max);
                    }
                    gst_object_unref (peer);
                }
                return TRUE;
            default:
                return GST_BASE_TRANSFORM_CLASS (gst_lgecnr_probe_parent_class)->query (trans, direction, query);
        }
    } else {
        return GST_BASE_TRANSFORM_CLASS (gst_lgecnr_probe_parent_class)->query (trans, direction, query);
    }
}

static void gst_lgecnr_probe_class_init (GstLgecnrProbeClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
    GstAudioFilterClass *audio_filter_class = GST_AUDIO_FILTER_CLASS (klass);

    /* Setting up pads and setting metadata should be moved to
    base_class_init if you intend to subclass this class. */

    gobject_class->set_property = gst_lgecnr_probe_set_property;
    gobject_class->get_property = gst_lgecnr_probe_get_property;

    gobject_class->finalize = gst_lgecnr_probe_finalize;
    base_transform_class->passthrough_on_same_caps = TRUE;
    base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_lgecnr_echo_probe_src_event);

    base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_lgecnr_probe_prepare_output_buffer);
    base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_lgecnr_echo_probe_stop);

    base_transform_class->query = GST_DEBUG_FUNCPTR(gst_lgecnr_probe_query);

    audio_filter_class->setup = GST_DEBUG_FUNCPTR (gst_lgecnr_probe_setup);
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get (&gst_lgecnr_probe_src_template));
    gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
                                        gst_static_pad_template_get (&gst_lgecnr_probe_sink_template));
    gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
                                           "LG ECNR Probe Plugin", "ECNR processing", "A plugin stores streaming data from udpsrc into buffer. A LGECNR plugin will retrieve the data.",
                                           "Hyobeom Lee <hyobeom1.lee@lge.com> & Dhilipkumar R <dhilipkumar.raman@lge.com>");
    g_object_class_install_property (gobject_class,
                                     PROP_TIMEADJUST,
                                     g_param_spec_uint64 ("timeadjust", "time adjust value",
                                                          "time adjust value for ecnr speaker out data",
                                                          0,
                                                          1000,
                                                          0,
                                                          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT)));
    g_object_class_install_property (gobject_class,
                                     PROP_LATENCY_ADJUST,
                                     g_param_spec_uint64 ("latencyadjust", "latcency adjustment value in ms",
                                                          "latcency adjustment value in ms for ecnr speaker out data",
                                                          0,
                                                          128,
                                                          10,
                                                          (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT)));


}

static void gst_lgecnr_probe_init (GstLgecnrProbe *lgecnrprobe)
{
    g_mutex_init (&lgecnrprobe->lock);

    G_LOCK (gst_lgecnr_probes);
    gst_lgecnr_probes = g_list_prepend (gst_lgecnr_probes, lgecnrprobe);
    G_UNLOCK (gst_lgecnr_probes);
}

GstLgecnrProbe * gst_lgecnr_acquire_echo_probe (const gchar * name)
{
    GstLgecnrProbe *ret = NULL;
    GList *l;

    G_LOCK (gst_lgecnr_probes);
    for (l = gst_lgecnr_probes; l; l = l->next) {
        GstLgecnrProbe *probe = GST_LGECNR_PROBE (l->data);

        GST_LGECNR_PROBE_LOCK (probe);
        if (!probe->acquired && g_strcmp0 (GST_OBJECT_NAME (probe), name) == 0) {
            probe->acquired = TRUE;
            ret = GST_LGECNR_PROBE (gst_object_ref (probe));
            GST_LGECNR_PROBE_UNLOCK (probe);
            break;
        }
        GST_LGECNR_PROBE_UNLOCK (probe);
    }
    G_UNLOCK (gst_lgecnr_probes);

    return ret;
}

void gst_lgecnr_release_echo_probe (GstLgecnrProbe * probe)
{
    GST_LGECNR_PROBE_LOCK (probe);
    probe->acquired = FALSE;
    GST_LGECNR_PROBE_UNLOCK (probe);
    gst_object_unref (probe);
}

void gst_lgecnr_probe_set_property (GObject * object, guint property_id,
                               const GValue * value, GParamSpec * pspec)
{
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

    GST_DEBUG_OBJECT (lgecnrprobe, "set_property");

    switch (property_id) {
        case PROP_TIMEADJUST:
            lgecnrprobe->timeadjust = g_value_get_uint64 (value);
            GST_INFO_OBJECT (lgecnrprobe, "set timeadjust =%" G_GUINT64_FORMAT, lgecnrprobe->timeadjust);
            break;
        case PROP_LATENCY_ADJUST:
            lgecnrprobe->latencyadjust = g_value_get_uint64 (value);
            GST_INFO_OBJECT (lgecnrprobe, "set latency adjust =%" G_GUINT64_FORMAT, lgecnrprobe->latencyadjust);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_lgecnr_probe_get_property (GObject * object, guint property_id,
                               GValue * value, GParamSpec * pspec)
{
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

    GST_DEBUG_OBJECT (lgecnrprobe, "get_property");

    switch (property_id) {
        case PROP_TIMEADJUST:
            g_value_set_uint64 (value, lgecnrprobe->timeadjust);
            GST_INFO_OBJECT (lgecnrprobe, "time adjust current val =%" G_GUINT64_FORMAT, lgecnrprobe->timeadjust);
            break;
        case PROP_LATENCY_ADJUST:
            g_value_set_uint64 (value, lgecnrprobe->latencyadjust);
            GST_INFO_OBJECT (lgecnrprobe, "latency adjust current val =%" G_GUINT64_FORMAT, lgecnrprobe->latencyadjust);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_lgecnr_probe_finalize (GObject * object)
{
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

    GST_DEBUG_OBJECT (lgecnrprobe, "finalize");

    /* clean up object here */

    G_LOCK (gst_lgecnr_probes);
    gst_lgecnr_probes = g_list_remove (gst_lgecnr_probes, lgecnrprobe);
    G_UNLOCK (gst_lgecnr_probes);

    lgecnrprobe->adapter = NULL;

    G_OBJECT_CLASS (gst_lgecnr_probe_parent_class)->finalize (object);
}

static gboolean gst_lgecnr_probe_setup (GstAudioFilter * filter, const GstAudioInfo * info)
{
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (filter);

    GST_DEBUG_OBJECT (lgecnrprobe, "setup");
    GST_LOG_OBJECT (lgecnrprobe, "setting format to %s with %i Hz and %i channels",
                    info->finfo->description, info->rate, info->channels);

    return TRUE;
}

static gboolean
gst_lgecnr_echo_probe_stop (GstBaseTransform * trans)
{
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);

    GST_DEBUG_OBJECT (lgecnrprobe, "stop");

    return TRUE;
}

static gboolean gst_lgecnr_echo_probe_src_event (GstBaseTransform * trans, GstEvent * event)
{
    GstBaseTransformClass *klass;
    GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);
    GstClockTime latency;
    GstClockTime upstream_latency = 0;
    GstQuery *query;

    klass = GST_BASE_TRANSFORM_CLASS (gst_lgecnr_probe_parent_class);

    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_LATENCY:
            gst_event_parse_latency (event, &latency);
            query = gst_query_new_latency ();

            if (gst_pad_query (trans->srcpad, query)) {
                gst_query_parse_latency (query, NULL, &upstream_latency, NULL);

                if (!GST_CLOCK_TIME_IS_VALID (upstream_latency))
                    upstream_latency = 0;
            }

            GST_LGECNR_PROBE_LOCK (lgecnrprobe);
            lgecnrprobe->latency = latency;
            lgecnrprobe->delay = upstream_latency / GST_MSECOND;
            GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);

            GST_DEBUG_OBJECT (lgecnrprobe, "We have a latency of %" GST_TIME_FORMAT
                    " and delay of %ims", GST_TIME_ARGS (latency),
                              (gint) (upstream_latency / GST_MSECOND));
            break;
        default:
            break;
    }

    return klass->src_event (trans, event);
}
