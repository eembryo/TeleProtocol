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
#include "gstlgecnrprobe.h"
#include <string.h>
#include <stdio.h>

GST_DEBUG_CATEGORY_STATIC (gst_lgecnr_debug_category);
#define GST_CAT_DEFAULT gst_lgecnr_debug_category

#ifdef ECNR_ENABLE
#define kECNRSampleSize_8K					(128)
#define kECNRSampleSize_16K_24K				(256)
#define ecnrConfigPath						"/app/gstlgecnr/config/"
#define libFilePath							"/app/lib/"
#define libFileFullPath						"/app/lib/libtse.so"

#define RING_BUFFER_RATIO 8
#define MONO_CHANNEL 1
#define STEREO_CHANNEL 2

#define TSEWarningCodeToString_( X ) \
    ( ( (X) == TSE_WARN_TSF_BUFFER_NULL )       ? "Buffer is Null"          : \
      ( (X) == TSE_WARN_TSF_CONVERT_FAILED )    ? "Convert failed"          : \
      ( (X) == TSE_WARN_TSF_CRC_CHECK_FAILED )  ? "CRC check failed"        : \
      ( (X) == TSE_WARN_TSF_DECODE_FAILED )     ? "Decode failed"           : \
      ( (X) == TSE_WARN_TSF_VERSION_MISMATCH )  ? "Version Mismatched"      : \
      ( (X) == TSE_WARN_TSF_FILE_OPEN_FAILED )  ? "Tsf file open failed"    : \
                                                  "Invalied warning code" )

#endif
/* prototypes */

static void gst_lgecnr_set_property (GObject * object,
		guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_lgecnr_get_property (GObject * object,
		guint property_id, GValue * value, GParamSpec * pspec);
static void gst_lgecnr_dispose (GObject * object);
static void gst_lgecnr_finalize (GObject * object);

static gboolean gst_lgecnr_setup (GstAudioFilter * filter,
		const GstAudioInfo * info);
static GstFlowReturn gst_lgecnr_transform (GstBaseTransform * trans,
		GstBuffer * inbuf, GstBuffer * outbuf);
static GstBuffer * gst_lgecnr_take_buffer (GstLgecnr * lgecnr);
static gboolean gst_lgecnr_stop(GstBaseTransform * trans);
static gboolean gst_lgecnr_start(GstBaseTransform * trans);

enum
{
	PROP_0,
	PROP_ECHO_CANCEL,
	PROP_PROBE_NAME
};

/* pad templates */

static GstStaticPadTemplate gst_lgecnr_src_template =
		GST_STATIC_PAD_TEMPLATE ("src",
				GST_PAD_SRC,
				GST_PAD_ALWAYS,
				GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=8000,"
						"channels=1,layout=interleaved")
		);

static GstStaticPadTemplate gst_lgecnr_sink_template =
		GST_STATIC_PAD_TEMPLATE ("sink",
				GST_PAD_SINK,
				GST_PAD_ALWAYS,
				GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=8000,"
						"channels=1,layout=interleaved")
		);


/* class initialization */
static GstFlowReturn
gst_lgecnr_prepare_output_buffer (GstBaseTransform *trans, GstBuffer *inBuf, GstBuffer **outBuf)
{
	GstLgecnr *lgecnr = GST_LGECNR (trans);
	GstBuffer *buffer;
	gsize buffer_size, adapter_size;

	// 1. push incoming buffer to adapter
	buffer = gst_buffer_copy (inBuf);
	GST_BUFFER_PTS (buffer) = gst_segment_to_running_time (&trans->segment,
			GST_FORMAT_TIME, GST_BUFFER_PTS (buffer));

	if (GST_BUFFER_IS_DISCONT (inBuf)) {
		GST_DEBUG_OBJECT (lgecnr,
				"Received discont, clearing adapter.");
		gst_adapter_clear (lgecnr->adapter);
	}

	gst_adapter_push (lgecnr->adapter, buffer);

	// 2. create buffer
	adapter_size = gst_adapter_available (lgecnr->adapter);
	if (adapter_size < lgecnr->period_size) {
		*outBuf = NULL;
		//return GST_FLOW_OK;
	} else {
		buffer_size = adapter_size - (adapter_size % lgecnr->period_size);
		*outBuf = gst_buffer_new_allocate (NULL, buffer_size, NULL);
	}
	return GST_FLOW_OK;
}

G_DEFINE_TYPE_WITH_CODE (GstLgecnr, gst_lgecnr, GST_TYPE_AUDIO_FILTER,
		GST_DEBUG_CATEGORY_INIT (gst_lgecnr_debug_category, "lgecnr", 0,
				"debug category for lgecnr element"));

static void
gst_lgecnr_class_init (GstLgecnrClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
	GstAudioFilterClass *audio_filter_class = GST_AUDIO_FILTER_CLASS (klass);

	/* Setting up pads and setting metadata should be moved to
	base_class_init if you intend to subclass this class. */

	gobject_class->set_property = GST_DEBUG_FUNCPTR(gst_lgecnr_set_property);
	gobject_class->get_property = GST_DEBUG_FUNCPTR(gst_lgecnr_get_property);
	gobject_class->dispose = GST_DEBUG_FUNCPTR(gst_lgecnr_dispose);
	gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_lgecnr_finalize);

	base_transform_class->passthrough_on_same_caps = FALSE;
	base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_lgecnr_transform);
	base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_lgecnr_prepare_output_buffer);
	base_transform_class->start = GST_DEBUG_FUNCPTR (gst_lgecnr_start);
	base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_lgecnr_stop);

	audio_filter_class->setup = GST_DEBUG_FUNCPTR (gst_lgecnr_setup);
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
			gst_static_pad_template_get (&gst_lgecnr_src_template));
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
			gst_static_pad_template_get (&gst_lgecnr_sink_template));

	gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
			"LG ECNR Plugin", "ECNR processing", "A plugin stores streaming data from audio devices into buffer. and collect data from LGECNR probe plugin and does ecnr processing.",
			"Hyobeom Lee <hyobeom1.lee@lge.com> & Dhilipkumar R <dhilipkumar.raman@lge.com>");

	g_object_class_install_property (gobject_class,
									PROP_PROBE_NAME,
									g_param_spec_string ("probe", "Echo Probe",
									"The name of the lgecnrprobe element that record the audio being "
									"played through loud speakers. Must be set before PAUSED state.",
									"lgecnrprobe0",
									(GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
									G_PARAM_CONSTRUCT)));

	g_object_class_install_property (gobject_class,
									PROP_ECHO_CANCEL,
									g_param_spec_boolean ("echoCancel", "Echo Cancel",
									"Enable or disable echo canceller, note that it will be disabled if "
									"no lgecnrprobe has been found", TRUE,
									(GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT)));

}

#ifdef ECNR_ENABLE


void setLibDynamicLoadingPath(GstLgecnr *lgecnr)
{
	int err = 0;
	const char *path = NULL;

	//path = LGTSE_GetLibraryFullPath(ecnrConfigPath, libFilePath);

	if (path == NULL)
	{
		GST_DEBUG_OBJECT (lgecnr, "GetLibraryFullPath NULL!, Set default!");
		path = libFileFullPath;
	}

	GST_DEBUG_OBJECT (lgecnr, "Library file full path :%s", path);

	err = LGTSE_StartDynamicLoading(path);
	if (err != TSE_ERR_OK)
	{
		// retry again using default full path name.
		GST_DEBUG_OBJECT (lgecnr, " %s is not found! Maybe version postfix is not used. %d", path, err);
		GST_DEBUG_OBJECT (lgecnr, "Retry to load lib %s", libFileFullPath);

		err = LGTSE_StartDynamicLoading(libFileFullPath);
		if (err != TSE_ERR_OK)
		{
			GST_DEBUG_OBJECT (lgecnr, "Lib DynamicLoading Error still!!!! %d", err);
		}
		else
		{
			GST_DEBUG_OBJECT (lgecnr, "Lib DynamicLoading success!!!! ");
		}
	}
}

int prepare(GstLgecnr *lgecnr)
{
	int err;
	const char *configpath = ecnrConfigPath;

	setLibDynamicLoadingPath(lgecnr);

	lgecnr->mTseCtx = LGTSE_FilterCreate();
	if(lgecnr->mTseCtx == NULL)
	{
		GST_DEBUG_OBJECT (lgecnr, "LGTSE_FilterCreate Error");
		err = -1;
		require(0 == err, exit);
	}
	err = LGTSE_SetTsfPath(lgecnr->mTseCtx, configpath);
	if (err != TSE_ERR_OK)
	{
		GST_DEBUG_OBJECT (lgecnr, "LGTSE_SetTsfPath Error");
	}
	err = LGTSE_FilterInit(lgecnr->mTseCtx, lgecnr->mConfigMode, lgecnr->mSampleRate, lgecnr->mMicInChannelCnt, lgecnr->mSampleFrames);
	if (err != TSE_ERR_OK)
	{
		if (err > TSE_WARN_START && err < TSE_WARN_END )
			GST_DEBUG_OBJECT (lgecnr, "prepare():LGTSE_FilterInit Error %s", TSEWarningCodeToString_(err));
		else
			GST_DEBUG_OBJECT (lgecnr, "prepare():LGTSE_FilterInit Error  Unknown");
	}
exit:
	return err;
}

void initialize_ecnrlib(GstLgecnr *lgecnr)
{
	lgecnr->mSampleFrames = (lgecnr->info.rate == 8000 ? kECNRSampleSize_8K : kECNRSampleSize_16K_24K); //PCM Raw data framesize, 16K : 256,  8K : 128
	lgecnr->mSampleBytes = lgecnr->mSampleFrames * lgecnr->info.bpf;
	lgecnr->mSampleRate = lgecnr->info.rate; //use lgecnr->info.rate instead of mSampleRate
	lgecnr->mConfigMode = 0x00; //CARPLAY_TELEPHONY_MODE
	//lgecnr->mConfigMode = 0x03; //TELEMATICS_MODE
	lgecnr->mBufferSize = lgecnr->mSampleBytes * RING_BUFFER_RATIO;
	lgecnr->mMicInChannelCnt = MONO_CHANNEL;
	prepare(lgecnr);
}

#endif
static void
gst_lgecnr_init (GstLgecnr *lgecnr)
{
	GST_DEBUG_OBJECT (lgecnr, "init");
	lgecnr->adapter = gst_adapter_new ();
	gst_audio_info_init (&lgecnr->info);
}

void
gst_lgecnr_set_property (GObject * object, guint property_id,
		const GValue * value, GParamSpec * pspec)
{
	GstLgecnr *lgecnr = GST_LGECNR (object);

	GST_DEBUG_OBJECT (lgecnr, "set_property");

	switch (property_id) {
		case PROP_ECHO_CANCEL:
			lgecnr->echoCancel = g_value_get_boolean (value);
			break;
		case PROP_PROBE_NAME:
			g_free (lgecnr->probe_name);
			lgecnr->probe_name = g_value_dup_string (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

void
gst_lgecnr_get_property (GObject * object, guint property_id,
		GValue * value, GParamSpec * pspec)
{
	GstLgecnr *lgecnr = GST_LGECNR (object);

	GST_DEBUG_OBJECT (lgecnr, "get_property");
	switch (property_id) {
		case PROP_ECHO_CANCEL:
			g_value_set_boolean (value, lgecnr->echoCancel);
			break;
		case PROP_PROBE_NAME:
			g_value_set_string (value, lgecnr->probe_name);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

void
gst_lgecnr_dispose (GObject * object)
{
	GstLgecnr *lgecnr = GST_LGECNR (object);

	GST_DEBUG_OBJECT (lgecnr, "dispose");

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS (gst_lgecnr_parent_class)->dispose (object);
}

void
gst_lgecnr_finalize (GObject * object)
{
	GstLgecnr *lgecnr = GST_LGECNR (object);

	GST_DEBUG_OBJECT (lgecnr, "finalize");

	/* clean up object here */
	 gst_object_unref (lgecnr->adapter);


#ifdef ECNR_ENABLE
	if(lgecnr->mTseCtx != NULL)
	{
		GST_DEBUG_OBJECT (lgecnr, " LGTSE_FilterDestroy!!");
		LGTSE_FilterDestroy(lgecnr->mTseCtx);
	}
#endif

	g_free (lgecnr->probe_name);
	G_OBJECT_CLASS (gst_lgecnr_parent_class)->finalize (object);
}

static gboolean
gst_lgecnr_setup (GstAudioFilter * filter, const GstAudioInfo * info)
{
	GstLgecnr *lgecnr = GST_LGECNR (filter);
	GstAudioInfo probe_info = *info;

	//gint err = 0;

	GST_DEBUG_OBJECT (lgecnr, "setting format to %s with %i Hz and %i channels",
			info->finfo->description, info->rate, info->channels);

	GST_OBJECT_LOCK (lgecnr);

	gst_adapter_clear (lgecnr->adapter);
	lgecnr->info = *info;

	// LGECNR library takes 16ms buffer.
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

#ifdef ECNR_ENABLE
	initialize_ecnrlib(lgecnr);
#endif

	GST_OBJECT_UNLOCK (lgecnr);

	return TRUE;

period_too_big:
	GST_OBJECT_UNLOCK (lgecnr);
	GST_WARNING_OBJECT (lgecnr, "lgecnr format produce too big period "
						"(maximum is %" G_GSIZE_FORMAT " samples and we have %u samples), "
						"reduce the number of channels or the rate.",
						(gsize)kMaxDataSizeSamples, lgecnr->period_size / 2);
	return FALSE;

probe_has_wrong_rate:
	GST_LGECNR_PROBE_UNLOCK (lgecnr->probe);
	GST_OBJECT_UNLOCK (lgecnr);
	GST_ELEMENT_ERROR (lgecnr, STREAM, FORMAT,
						("lgecnr echo Probe has rate %i , while the lgecnr plugin is running at rate %i,"
						" use a caps filter to ensure those are the same.",
						probe_info.rate, info->rate), (NULL));
	return FALSE;
}

/* transform */
static GstFlowReturn
gst_lgecnr_transform (GstBaseTransform * trans, GstBuffer * inBuf,
		GstBuffer * outBuf)
{
	GstLgecnr *lgecnr = GST_LGECNR (trans);
	GstClockTime timestamp;
	guint64 distance;
	guint n_periods;
	GstLgecnrProbe *ecnr_probe = NULL;
	AudioFrame frame = {{0},0,0,0};
	GstFlowReturn ret = GST_FLOW_OK;
	int err;
	GST_DEBUG_OBJECT (lgecnr, "transform: inbuf = 0x%p, outbuf = 0x%p",inBuf, outBuf);

	n_periods = gst_adapter_available (lgecnr->adapter) / lgecnr->period_size;

	{
		GstMapInfo mapinfo;
		gconstpointer InputData;
		gst_buffer_map (outBuf, &mapinfo, GST_MAP_WRITE);
		if (n_periods*lgecnr->period_size > mapinfo.maxsize) {
			GST_ERROR_OBJECT (lgecnr, "generated buffer size (%d) is not enough for (%d).", (int)(mapinfo.maxsize), n_periods*lgecnr->period_size);
			return GST_FLOW_ERROR;
		}

#ifdef ECNR_ENABLE
		timestamp = gst_adapter_prev_pts (lgecnr->adapter, &distance);
		if (timestamp == GST_CLOCK_TIME_NONE && distance == 0) {
			GST_BUFFER_FLAG_SET (outBuf, GST_BUFFER_FLAG_DISCONT);
		} else
			GST_BUFFER_FLAG_UNSET (outBuf, GST_BUFFER_FLAG_DISCONT);
		timestamp += gst_util_uint64_scale_int (distance / lgecnr->info.bpf, GST_SECOND, lgecnr->info.rate);
		GST_BUFFER_PTS (outBuf) = timestamp;
		GST_BUFFER_DURATION (outBuf) = 16 * n_periods * GST_MSECOND;

		InputData = gst_adapter_map(lgecnr->adapter, n_periods*lgecnr->period_size);
		GST_OBJECT_LOCK (lgecnr);
		if (lgecnr->echoCancel)	//enable disable echo cancellation
			ecnr_probe = GST_LGECNR_PROBE (g_object_ref (lgecnr->probe));
		GST_OBJECT_UNLOCK (lgecnr);

		if (lgecnr->echoCancel)	//enable disable echo cancellation
			ecnr_probe = GST_LGECNR_PROBE (g_object_ref (lgecnr->probe));

		if (!ecnr_probe)
			return GST_FLOW_OK;

		char outpuAudioBuffer [lgecnr->period_size];
		GST_INFO_OBJECT (lgecnr, "looping = %d", n_periods);
		for (int i=0; i<n_periods;i++) {
			gst_lgecnr_echo_probe_read (ecnr_probe, timestamp + 16*GST_MSECOND*i, (gpointer) &frame);
			//gst_lgecnr_echo_probe_read (ecnr_probe, timestamp, (gpointer) &frame);

			LGTSE_FilterProcess(lgecnr->mTseCtx, (char*)InputData, (char*)mapinfo.data, (char*)&frame.data_, (char*)outpuAudioBuffer, NULL);
			InputData+=lgecnr->period_size;
			mapinfo.data+=lgecnr->period_size;
			GST_INFO_OBJECT (lgecnr, "---------------------");
		}
		GST_DEBUG_OBJECT (lgecnr, "audioEcnr prcoessing successful " );
		gst_buffer_unmap (outBuf, &mapinfo);
		gst_adapter_flush (lgecnr->adapter, n_periods * lgecnr->period_size);
done:
	gst_object_unref (ecnr_probe);
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

static gboolean
gst_lgecnr_stop(GstBaseTransform * trans)
{
	GstLgecnr *lgecnr = GST_LGECNR (trans);

	GST_OBJECT_LOCK (lgecnr);

	gst_adapter_clear (lgecnr->adapter);

	if (lgecnr->probe) {
		gst_lgecnr_release_echo_probe (lgecnr->probe);
		lgecnr->probe = NULL;
	}

	GST_OBJECT_UNLOCK (lgecnr);

	return TRUE;
}

static gboolean
gst_lgecnr_start (GstBaseTransform * trans)
{
	GstLgecnr *lgecnr = GST_LGECNR (trans);
	 GST_OBJECT_LOCK (lgecnr);

	 if (lgecnr->echoCancel) // enable disable echo cancelling
		 lgecnr->probe = gst_lgecnr_acquire_echo_probe (lgecnr->probe_name);

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
plugin_init (GstPlugin * plugin)
{
	if (!gst_element_register (plugin, "lgecnrprobe", GST_RANK_NONE, GST_TYPE_LGECNR_PROBE)) {
		return FALSE;
	}

	if (!gst_element_register (plugin, "lgecnr", GST_RANK_NONE,	GST_TYPE_LGECNR)) {
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

