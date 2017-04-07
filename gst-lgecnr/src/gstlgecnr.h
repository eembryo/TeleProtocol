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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_LGECNR_H_
#define _GST_LGECNR_H_

#include <gst/audio/gstaudiofilter.h>
#include "gstlgecnrprobe.h"
#include "commontypes.h"
#include <gst/base/gstadapter.h>
#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/audio/audio.h>

#ifdef ECNR_ENABLE
#include "ringbuffer.h"
#include "TseEcnrDebug.h"
#include "tseFilter.h"
#include "tseFilter_priv.h"
#endif

G_BEGIN_DECLS

#define GST_TYPE_LGECNR   (gst_lgecnr_get_type())
#define GST_LGECNR(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_LGECNR,GstLgecnr))
#define GST_LGECNR_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_LGECNR,GstLgecnrClass))
#define GST_IS_LGECNR(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_LGECNR))
#define GST_IS_LGECNR_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_LGECNR))

typedef struct _GstLgecnr GstLgecnr;
typedef struct _GstLgecnrClass GstLgecnrClass;

struct _GstLgecnr
{
	GstAudioFilter base_lgecnr;

	/* Protected by the object lock */
	GstAudioInfo info;
	guint period_size;

	/* Protected by the stream lock */
	GstAdapter *adapter;
	//lge ecnr library pointer;

	/* Protected by the object lock */
	gchar *probe_name;
	GstLgecnrProbe *probe;

	gboolean echoCancel;
#ifdef ECNR_ENABLE
	gsize mSampleBytes;
	gsize mSampleRate;
	gsize mBufferSize;
	gsize mSampleFrames;
	gsize mMicInChannelCnt;
	gint8 mConfigMode;
	void *mTseCtx;
#endif
};

struct _GstLgecnrClass
{
	GstAudioFilterClass base_lgecnr_class;
};

GType gst_lgecnr_get_type (void);

G_END_DECLS

#endif	//_GST_LGECNR_H_
