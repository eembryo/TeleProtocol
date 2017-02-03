
/*
 * @@@ LICENSE
 * Copyright (C) 2015-2016, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 * Design/Author : dhilipkumar.raman@lge.com, jinheung.tark@lgepartner.com
 * Date : 02/04/2016
 *
 * <IHU XCallService> Detailed note is as follows.
 *
 *
 */

#ifndef __TEMXCALL_UNITTEST_RTP_PIPELINE_H__
#define __TEMXCALL_UNITTEST_RTP_PIPELINE_H__

#include <memory>
#include <glib.h>
#include <gio/gio.h>


#include <gst/gst.h>

#include "abstract_pipeline.h"
#include "xcall_export.h"

namespace temxcall {

class UnitTestRTPPipeline;

typedef struct _CustomData {
	gboolean is_live;
	GstElement *pipeline;
	GMainLoop *loop;
	gulong id; // probe ID
	GstElement *udpSrc1;
	GstElement *rtpBin;
	GstElement *rtpSpeexDepay;
	GstElement *speexDec;
	GstElement *audioConvert;
	GstElement *audioReSample;
	GstElement *autoAudioSink;
	UnitTestRTPPipeline *thisPtr;
} CustomData;

class UnitTestRTPPipeline : public AbstractPipeline  {

public:
	UnitTestRTPPipeline();
	~UnitTestRTPPipeline();

	virtual bool Load();
	virtual bool Play();
	virtual bool Unload();
	virtual bool Pause();
	virtual bool Resume();

//  static gboolean gstBusCallbackHandle (GstBus * pBus, GstMessage * pMessage, gpointer data);

	static void on_pad_added1 (GstElement *element, GstPad *pad, gpointer data);
	static void on_pad_added2 (GstElement *element, GstPad *pad, gpointer data);
	static gboolean bus_call_back (GstBus *bus, GstMessage *msg, gpointer data);
	static GstPadProbeReturn pad_probe_cb (GstPad * pad, GstPadProbeInfo * info, gpointer data);


#ifdef DEBUG_ENABLED
	virtual void setSourceInformation(xcall_debug_input input);
#endif

protected:

	bool InitReceiverPipeline ();
	bool InitSenderPipeline();

private:

	GstElement *pipeline;

	// receiving elements
	GstElement *udpSrc1;
	GstElement *rtpBin;
	GstElement *rtpSpeexDepay;
	GstElement *speexDec;
	GstElement *audioConvert;
	GstElement *audioReSample;
	GstElement *autoAudioSink;
	GstElement *udpSrc2;
	GstElement *udpSrcSink;

	// sending element
	GstElement *fileSrc;
	GstElement *Mad;
	GstElement *audioConvertSend;
	GstElement *speexEnc;
	GstElement *rtpSpeexPay;
	GstElement *udpSink1;
	GstElement *udpSink2;
	GstElement *udpSinkSrc;

	GstBus *bus;

	guint bus_watch_id;

	GMainLoop *mainLoop;
	CustomData data;

	xcall_debug_input input;
};

} //temxcall
#endif //__TEMXCALL_UNITTEST_RTP_PIPELINE_H__

