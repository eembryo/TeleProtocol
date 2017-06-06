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
static GstFlowReturn gst_lgecnr_probe_transform_ip (GstBaseTransform * trans, GstBuffer * buf);
static gboolean gst_lgecnr_echo_probe_src_event (GstBaseTransform * trans, GstEvent * event);
static gboolean gst_lgecnr_echo_probe_stop (GstBaseTransform * trans);
static GstFlowReturn gst_lgecnr_echo_transform (GstBaseTransform * trans,
		GstBuffer * inbuf, GstBuffer * outbuf);

enum
{
	PROP_0,
	PROP_TIMEADJUST
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


static GstFlowReturn
gst_lgecnr_probe_prepare_output_buffer (GstBaseTransform *trans, GstBuffer *inBuf, GstBuffer **outBuf)
{
	GstClockTime timestamp;
	guint64 distance;
	guint n_periods;
	gsize buffer_size, adapter_size;
	GstBuffer *newbuf = NULL;
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);

	// push input buffer to adapter
	GST_LGECNR_PROBE_LOCK (lgecnrprobe);
	newbuf = gst_buffer_copy (inBuf);
	/* Moves the buffer timestamp to be in Running time */
	GST_INFO_OBJECT (lgecnrprobe, "inBuf at prepare outout buffer timestamp = %"G_GUINT64_FORMAT, GST_BUFFER_PTS (inBuf));
	GST_BUFFER_PTS (newbuf) = gst_segment_to_running_time (&trans->segment,
	GST_FORMAT_TIME, GST_BUFFER_PTS (inBuf));
	gst_adapter_push (lgecnrprobe->adapter, newbuf);

	if (gst_adapter_available (lgecnrprobe->adapter) > MAX_ADAPTER_SIZE)
		gst_adapter_flush (lgecnrprobe->adapter,
			gst_adapter_available (lgecnrprobe->adapter) - MAX_ADAPTER_SIZE);

	//create output buffer
	adapter_size = gst_adapter_available (lgecnrprobe->spkAdapter);
	if (adapter_size < lgecnrprobe->period_size) {
		*outBuf = NULL;
	} else {
		buffer_size = adapter_size - (adapter_size % lgecnrprobe->period_size);
		*outBuf = gst_buffer_new_allocate (NULL, buffer_size, NULL);
	}

	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);
	return GST_FLOW_OK;
}

static void
gst_lgecnr_probe_class_init (GstLgecnrProbeClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
	GstAudioFilterClass *audio_filter_class = GST_AUDIO_FILTER_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	base_class_init if you intend to subclass this class. */

	gobject_class->set_property = gst_lgecnr_probe_set_property;
	gobject_class->get_property = gst_lgecnr_probe_get_property;
	//gobject_class->dispose = gst_lgecnr_probe_dispose;
	gobject_class->finalize = gst_lgecnr_probe_finalize;
	base_transform_class->passthrough_on_same_caps = FALSE;
	base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_lgecnr_echo_probe_src_event);
	base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_lgecnr_echo_transform);
	base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_lgecnr_probe_prepare_output_buffer);
	base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_lgecnr_echo_probe_stop);
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


}

static void
gst_lgecnr_probe_init (GstLgecnrProbe *lgecnrprobe)
{
	lgecnrprobe->adapter = gst_adapter_new();
	lgecnrprobe->spkAdapter = gst_adapter_new();
	gst_audio_info_init (&lgecnrprobe->info);
	g_mutex_init (&lgecnrprobe->lock);
	lgecnrprobe->latency = GST_CLOCK_TIME_NONE;

	G_LOCK (gst_lgecnr_probes);
	gst_lgecnr_probes = g_list_prepend (gst_lgecnr_probes, lgecnrprobe);
	G_UNLOCK (gst_lgecnr_probes);
}

GstLgecnrProbe *
gst_lgecnr_acquire_echo_probe (const gchar * name)
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

void
gst_lgecnr_release_echo_probe (GstLgecnrProbe * probe)
{
	GST_LGECNR_PROBE_LOCK (probe);
	probe->acquired = FALSE;
	GST_LGECNR_PROBE_UNLOCK (probe);
	gst_object_unref (probe);
}

void
gst_lgecnr_probe_set_property (GObject * object, guint property_id,
		const GValue * value, GParamSpec * pspec)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

	GST_DEBUG_OBJECT (lgecnrprobe, "set_property");

	switch (property_id) {
		case PROP_TIMEADJUST:
			lgecnrprobe->timeadjust = g_value_get_uint64 (value);
			GST_INFO_OBJECT (lgecnrprobe, "set timeadjust =%"G_GUINT64_FORMAT, lgecnrprobe->timeadjust);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

void
gst_lgecnr_probe_get_property (GObject * object, guint property_id,
		GValue * value, GParamSpec * pspec)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

	GST_DEBUG_OBJECT (lgecnrprobe, "get_property");

	switch (property_id) {
		case PROP_TIMEADJUST:
			g_value_set_uint64 (value, lgecnrprobe->timeadjust);
			GST_INFO_OBJECT (lgecnrprobe, "time adjust current val =%"G_GUINT64_FORMAT, lgecnrprobe->timeadjust);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

void
gst_lgecnr_probe_dispose (GObject * object)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

	GST_DEBUG_OBJECT (lgecnrprobe, "dispose");

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS (gst_lgecnr_probe_parent_class)->dispose (object);
}

void
gst_lgecnr_probe_finalize (GObject * object)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (object);

	GST_DEBUG_OBJECT (lgecnrprobe, "finalize");

	/* clean up object here */

	G_LOCK (gst_lgecnr_probes);
	gst_lgecnr_probes = g_list_remove (gst_lgecnr_probes, lgecnrprobe);
	G_UNLOCK (gst_lgecnr_probes);

	gst_object_unref (lgecnrprobe->adapter);
	lgecnrprobe->adapter = NULL;

	gst_object_unref (lgecnrprobe->spkAdapter);
	lgecnrprobe->spkAdapter = NULL;

	G_OBJECT_CLASS (gst_lgecnr_probe_parent_class)->finalize (object);
}

static gboolean
gst_lgecnr_probe_setup (GstAudioFilter * filter, const GstAudioInfo * info)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (filter);

	GST_DEBUG_OBJECT (lgecnrprobe, "setup");
	GST_LOG_OBJECT (lgecnrprobe, "setting format to %s with %i Hz and %i channels",
					info->finfo->description, info->rate, info->channels);
	GST_LGECNR_PROBE_LOCK (lgecnrprobe);

	lgecnrprobe->info = *info;

	/* LGE ecnr library works with 16ms buffers, compute once this size */
	lgecnrprobe->period_size = info->bpf * info->rate * 0.016;//info->bpf * info->rate / 100;

	if ((lgecnrprobe->info.bpf == 0) || (lgecnrprobe->period_size ==0 ) || (lgecnrprobe->info.rate ==0))
	{
		lgecnrprobe->info.rate = 8000;
		lgecnrprobe->info.bpf =2;
		lgecnrprobe->period_size = 256;
	}

	if ((kMaxDataSizeSamples * 2) < lgecnrprobe->period_size)
		goto period_too_big;

	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);

	return TRUE;

	period_too_big:
	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);
	GST_WARNING_OBJECT (lgecnrprobe, "lgecnr format produce too big period "
						"(maximum is %" G_GSIZE_FORMAT " samples and we have %u samples), "
						"reduce the number of channels or the rate.",
						(gsize)kMaxDataSizeSamples, lgecnrprobe->period_size / 2);
	return FALSE;
}

/* transform */
static GstFlowReturn
gst_lgecnr_probe_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);

	GST_DEBUG_OBJECT (lgecnrprobe, "transform_ip");
	GstBuffer *newbuf = NULL;

	GST_LGECNR_PROBE_LOCK (lgecnrprobe);
	newbuf = gst_buffer_copy (buf);
	/* Moves the buffer timestamp to be in Running time */
	GST_BUFFER_PTS (newbuf) = gst_segment_to_running_time (&trans->segment,
	GST_FORMAT_TIME, GST_BUFFER_PTS (buf));
	gst_adapter_push (lgecnrprobe->adapter, newbuf);

	if (gst_adapter_available (lgecnrprobe->adapter) > MAX_ADAPTER_SIZE)
	gst_adapter_flush (lgecnrprobe->adapter,
		gst_adapter_available (lgecnrprobe->adapter) - MAX_ADAPTER_SIZE);
	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);

	return GST_FLOW_OK;
}

static GstFlowReturn
gst_lgecnr_echo_transform (GstBaseTransform * trans, GstBuffer * inBuf,
		GstBuffer * outBuf)
{
	GstClockTime timestamp;
	guint64 distance;
	guint n_periods;
	gsize buffer_size, copySize;
	GstBuffer *newbuf = NULL;
	GstMapInfo mapinfo;
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);

	GST_DEBUG_OBJECT (lgecnrprobe, "transform");

	if ((lgecnrprobe->info.bpf == 0) || (lgecnrprobe->period_size ==0 ) || (lgecnrprobe->info.rate ==0))
	{
		lgecnrprobe->info.rate = 8000;
		lgecnrprobe->info.bpf =2;
		lgecnrprobe->period_size = 256;
	}

	GST_LGECNR_PROBE_LOCK (lgecnrprobe);
	GST_DEBUG_OBJECT (lgecnrprobe, "inBuf at transform timestamp = %"G_GUINT64_FORMAT, GST_BUFFER_PTS (inBuf));

	/* speaker part*/
	buffer_size = gst_buffer_get_size (outBuf);
	if (buffer_size >= lgecnrprobe->period_size) {
		n_periods = buffer_size / lgecnrprobe->period_size;
		gst_buffer_map (outBuf, &mapinfo, GST_MAP_WRITE);
		if (n_periods*lgecnrprobe->period_size > mapinfo.maxsize) {
			GST_INFO_OBJECT (lgecnrprobe, "generated buffer size (%d) is not enough for (%d).", (int)(mapinfo.maxsize), n_periods*lgecnrprobe->period_size);
			return GST_FLOW_ERROR;
		}
		copySize = n_periods * lgecnrprobe->period_size;
		gst_adapter_copy (lgecnrprobe->spkAdapter, mapinfo.data, 0, copySize);
		gst_buffer_unmap (outBuf, &mapinfo);

		timestamp = gst_adapter_prev_pts (lgecnrprobe->spkAdapter, &distance);
		if (timestamp == GST_CLOCK_TIME_NONE && distance == 0) {
			GST_BUFFER_FLAG_SET (outBuf, GST_BUFFER_FLAG_DISCONT);
		} else
			GST_BUFFER_FLAG_UNSET (outBuf, GST_BUFFER_FLAG_DISCONT);
		timestamp += gst_util_uint64_scale_int (distance / lgecnrprobe->info.bpf, GST_SECOND, lgecnrprobe->info.rate);
		GST_INFO_OBJECT (lgecnrprobe, "outBuf timestamp = %"G_GUINT64_FORMAT, timestamp);

		if(lgecnrprobe->timeadjust)
			timestamp += lgecnrprobe->timeadjust*GST_SECOND;

		GST_BUFFER_PTS (outBuf) = timestamp;
		GST_BUFFER_DURATION (outBuf) = 16 * n_periods * GST_MSECOND;
		GST_INFO_OBJECT (lgecnrprobe, " modified timestamp = %"G_GUINT64_FORMAT, timestamp);
		gst_adapter_flush (lgecnrprobe->spkAdapter, copySize);
	}
	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);

	return GST_FLOW_OK;
}

static gboolean
gst_lgecnr_echo_probe_stop (GstBaseTransform * trans)
{
	GstLgecnrProbe *lgecnrprobe = GST_LGECNR_PROBE (trans);

	GST_DEBUG_OBJECT (lgecnrprobe, "stop");

	GST_LGECNR_PROBE_LOCK (lgecnrprobe);
	gst_adapter_clear (lgecnrprobe->adapter);
	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);
	return TRUE;
}

gint
gst_lgecnr_echo_probe_read (GstLgecnrProbe *lgecnrprobe, GstClockTime rec_time,
							gpointer _frame)
{
	AudioFrame * frame = (AudioFrame *) _frame;
	if ((lgecnrprobe->info.bpf == 0) || (lgecnrprobe->period_size ==0 ) || (lgecnrprobe->info.rate ==0))
	{
		lgecnrprobe->info.rate = 8000;
		lgecnrprobe->info.bpf =2;
		lgecnrprobe->period_size = 256;
	}
	GstClockTimeDiff diff;
	gsize avail, skip, offset, size;
	gint delay = -1;

	GST_LGECNR_PROBE_LOCK (lgecnrprobe);

	if (!GST_CLOCK_TIME_IS_VALID (lgecnrprobe->latency))
		goto done;

	/* In delay agnostic mode, just return 10ms of data */
	if (!GST_CLOCK_TIME_IS_VALID (rec_time)) {
		avail = gst_adapter_available (lgecnrprobe->adapter);

		if (avail < lgecnrprobe->period_size)
			goto done;

		size = lgecnrprobe->period_size;
		skip = 0;
		offset = 0;

		goto copy;
	}

	if (gst_adapter_available (lgecnrprobe->adapter) == 0) {
		diff = G_MAXINT64;
	} else {
		GstClockTime play_time;
		guint64 distance;

		play_time = gst_adapter_prev_pts (lgecnrprobe->adapter, &distance);

		if (GST_CLOCK_TIME_IS_VALID (play_time)) {
			play_time += gst_util_uint64_scale_int (distance / lgecnrprobe->info.bpf,
			GST_SECOND, lgecnrprobe->info.rate);
			play_time += lgecnrprobe->latency;

			diff = GST_CLOCK_DIFF (rec_time, play_time) / GST_MSECOND;
		} else {
			/* We have no timestamp, assume perfect delay */
			diff = lgecnrprobe->delay;
		}
		frame->timestamp = play_time;
	}

	avail = gst_adapter_available (lgecnrprobe->adapter);

	if (diff > lgecnrprobe->delay) {
		skip = (diff - lgecnrprobe->delay) * lgecnrprobe->info.rate / 1000 * lgecnrprobe->info.bpf;
		skip = MIN (lgecnrprobe->period_size, skip);
		offset = 0;
	} else {
		skip = 0;
		offset = (lgecnrprobe->delay - diff) * lgecnrprobe->info.rate / 1000 * lgecnrprobe->info.bpf;
		offset = MIN (avail, offset);
	}

	size = MIN (avail - offset, lgecnrprobe->period_size - skip);

	if (size < lgecnrprobe->period_size)
		memset (frame->data_, 0, lgecnrprobe->period_size);

copy:
	if (size) {
		gst_adapter_copy (lgecnrprobe->adapter, (guint8 *) frame->data_ + skip,
							offset, size);
		gst_adapter_flush (lgecnrprobe->adapter, offset + size);
		frame->size = size;
	}

	frame->num_channels_ = lgecnrprobe->info.channels;
	frame->sample_rate_hz_ = lgecnrprobe->info.rate;
	frame->samples_per_channel_ = lgecnrprobe->period_size / lgecnrprobe->info.bpf;

	delay = lgecnrprobe->delay;

done:
	GST_LGECNR_PROBE_UNLOCK (lgecnrprobe);

	return delay;
}

/*static gboolean
plugin_init (GstPlugin * plugin)
{
	if (!gst_element_register (plugin, "lgecnrprobe", GST_RANK_NONE, GST_TYPE_LGECNR_PROBE)) {
		return FALSE;
	}

	return TRUE;
}*/

static gboolean
gst_lgecnr_echo_probe_src_event (GstBaseTransform * trans, GstEvent * event)
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
