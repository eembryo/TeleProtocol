

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

#ifndef __TEMXCALL_UNITTEST_SPK_PIPELINE_H__
#define __TEMXCALL_UNITTEST_SPK_PIPELINE_H__

#include <memory>
#include <glib.h>
#include <gio/gio.h>

#include <gst/gst.h>

#include "abstract_pipeline.h"
#include "xcall_export.h"

namespace temxcall {

class UnitTestSPKPipeline : public AbstractPipeline  {

public:
	UnitTestSPKPipeline();
	~UnitTestSPKPipeline();

	virtual bool Load();
	virtual bool Play();
	virtual bool Unload();
	virtual bool Pause();
	virtual bool Resume();

#ifdef DEBUG_ENABLED
	virtual void setSourceInformation(xcall_debug_input input);
#endif

	static gboolean gstBusCallbackHandle (GstBus * pBus, GstMessage * pMessage, gpointer data);

	GstElement *m_pipeHandle;
	GstBus *m_busHandler;
	gulong m_sigBusAsync;

	GstElement *sink;
	GstElement *source;
	GstElement *decoder;
//	GstElement *convert; not used
protected:

};

}   //temxcall

#endif //__TEMXCALL_UNITTEST_SPK_PIPELINE_H__

