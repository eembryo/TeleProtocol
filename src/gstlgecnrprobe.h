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

#ifndef _GST_LGECNR_PROBE_H_
#define _GST_LGECNR_PROBE_H_

#include <gst/audio/gstaudiofilter.h>
#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <gst/base/gstbasetransform.h>
#include <gst/audio/audio.h>

G_BEGIN_DECLS

#define GST_TYPE_LGECNR_PROBE   (gst_lgecnr_probe_get_type())
#define GST_LGECNR_PROBE(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_LGECNR_PROBE,GstLgecnrProbe))
#define GST_LGECNR_PROBE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_LGECNR_PROBE,GstLgecnrProbeClass))
#define GST_IS_LGECNR_PROBE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_LGECNR_PROBE))
#define GST_IS_LGECNR_PROBE_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_LGECNR_PROBE))
#define GST_LGECNR_PROBE_LOCK(obj) g_mutex_lock (&GST_LGECNR_PROBE (obj)->lock)
#define GST_LGECNR_PROBE_UNLOCK(obj) g_mutex_unlock (&GST_LGECNR_PROBE (obj)->lock)

typedef struct _GstLgecnrProbe GstLgecnrProbe;
typedef struct _GstLgecnrProbeClass GstLgecnrProbeClass;

struct _GstLgecnrProbe
{
	GstAudioFilter base_lgecnrprobe;

	GMutex lock;
	/* Protected by the lock */
	GstAudioInfo info;
	guint period_size;
	GstClockTime latency;
	gint delay;
	guint64 timeadjust;

	GstSegment segment;
	GstAdapter *adapter;
	GstAdapter *spkAdapter;

	/* Private */
	gboolean acquired;
};

struct _GstLgecnrProbeClass
{
	GstAudioFilterClass base_lgecnrprobe_class;
};

GType gst_lgecnr_probe_get_type (void);
gint gst_lgecnr_echo_probe_read (GstLgecnrProbe * self, GstClockTime rec_time, gpointer frame);
GstLgecnrProbe *gst_lgecnr_acquire_echo_probe (const gchar * name);
void gst_lgecnr_release_echo_probe (GstLgecnrProbe * probe);

G_END_DECLS

#endif	//_GST_LGECNR_PROBE_H_
