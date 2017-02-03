

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

#ifndef __TEMXCALL_VOICECALL_PIPELINE_H__
#define __TEMXCALL_VOICECALL_PIPELINE_H__

#include <memory>
#include <string>
#include <map>
#include "abstract_pipeline.h"


namespace temxcall {

class VoicecallPipeline : public AbstractPipeline  {

public:
	VoicecallPipeline();
	virtual ~VoicecallPipeline();

	virtual bool Load();
	virtual bool Play();
	virtual bool Unload();
	virtual bool Pause();
	virtual bool Resume();
#ifdef DEBUG_ENABLED
	virtual void setSourceInformation(xcall_debug_input input);
#endif

	static void on_pad_added (GstElement *element, GstPad *pad, gpointer userData);
	static void on_pad_removed (GstElement *element, GstPad *pad, gpointer userData);
	static gboolean bus_call_back (GstBus *bus, GstMessage *msg, gpointer userData);
protected:
	bool InitReceiverPipeline ();
	bool InitSenderPipeline();

private:
	GstElement  *m_pipeline;

	// receiving elements
	GstElement  *m_rtpBin;
	GstCaps     *m_caps;
	GstElement  *m_udpSrc;
	GstElement  *m_inputSelector;
	GstElement  *m_rtpDepay;
	GstElement  *m_audioDec;
	GstElement  *m_audioConvert;
	GstElement  *m_audioReSample;
	GstElement  *m_audioSink;

	// sending element
	GstElement  *m_sourceEle;
	GstElement  *m_audioConvertSend;
	GstElement  *m_audioEnc;
	GstElement  *m_rtpPay;
	GstElement  *m_udpSink;

	GstBus      *m_bus;
	guint       m_bus_watch_id;

#if RECEIVE_FILESINK_ENABLED
	GstElement *fileSink;
	GstElement *wavEnc;
#endif

	std::map<GstPad*,GstPad*>   m_dynamicPadPairs;
	xcall_debug_input	m_input;
};

}	//temxcall

#endif //__TEMXCALL_VOICECALL_PIPELINE_H__

