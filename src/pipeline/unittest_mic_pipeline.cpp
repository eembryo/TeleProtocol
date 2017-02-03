

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

#include "unittest_mic_pipeline.h"

namespace temxcall {

/**
 * ================================================================================
 * @fn : UnitTestMICPipeline
 * @brief : This is constructor function for UnitTestMICPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestMICPipeline::UnitTestMICPipeline() {

	m_pipeHandle = NULL;
	m_busHandler = NULL;
	m_sigBusAsync = 0;

	source = encoder = muxer = sink = NULL;

}

/**
 * ================================================================================
 * @fn : ~UnitTestMICPipeline
 * @brief : This is destructor function for UnitTestMICPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestMICPipeline::~UnitTestMICPipeline() {

}

/**
 * ================================================================================
 * @fn : Load
 * @brief : This function loads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - start a main loop
 * - creates a pipline object
 * - creates the necessary elements
 * - requests for Gstreamer bus
 * - sets the pipeline status as GST_STATE_PAUSED
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestMICPipeline::Load()
{

	const gchar *filepath = "unittestmic.wav";

	XCALLLOGD("[UnitTestMICPipeline] Load with alsa-alsasink \n");

	gst_init(0,0);

	m_pipeHandle = gst_pipeline_new("micunittest");

	source = gst_element_factory_make("alsasrc" /*"autoaudiosrc"*/ /*"alsasrc"*/ /*"pulsesrc"*/, "source");

//	if (source){
//	  g_object_set(G_OBJECT(source), "device", "pcm_input", NULL);
//	}

#if 1
#if 1
	if (source){
		g_object_set(G_OBJECT(source), "device", "hw:3,0", NULL);
	}

	encoder = gst_element_factory_make("wavenc" , "encoder");

	sink = gst_element_factory_make("filesink", "sink");
	g_object_set(G_OBJECT(sink), "location", filepath, NULL);

	gst_bin_add_many(GST_BIN(m_pipeHandle), source, encoder, sink, NULL);
	gst_element_link_many(source, encoder, sink, NULL);

	gst_element_set_state (m_pipeHandle, GST_STATE_PAUSED);
#endif

	sink = gst_element_factory_make("filesink", "sink");
	g_object_set(G_OBJECT(sink), "location", filepath, NULL);

	gst_bin_add_many(GST_BIN(m_pipeHandle), source, sink, NULL);
	gst_element_link_many(source, sink, NULL);

	gst_element_set_state (m_pipeHandle, GST_STATE_PAUSED);

#else
	encoder = gst_element_factory_make("amrnbenc" /*"omxaacenc"*/, "encoder");
	muxer = gst_element_factory_make("3gppmux", "muxer");

	sink = gst_element_factory_make("filesink", "sink");
	g_object_set(G_OBJECT(sink), "location", filepath, NULL);

	gst_bin_add_many(GST_BIN(m_pipeHandle), source, encoder, muxer, sink, NULL);
	gst_element_link_many(source, encoder, muxer, sink, NULL);

	gst_element_set_state (m_pipeHandle, GST_STATE_PAUSED);
#endif

	if (m_pipeHandle) {
		XCALLLOGD("created micunittest pipeline \n");

		m_busHandler = gst_pipeline_get_bus(GST_PIPELINE(m_pipeHandle));

		if (m_busHandler) {
			gst_bus_add_signal_watch(m_busHandler);
			m_sigBusAsync = g_signal_connect(G_OBJECT(m_busHandler), "message",
									G_CALLBACK(gstBusCallbackHandle), this);
		}
	}

	return true;
}

/**
 * ================================================================================
 * @fn : Play
 * @brief : This function starts playing the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - sets the pipeline status as GST_STATE_PLAYING
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestMICPipeline::Play()
{

	XCALLLOGD("[UnitTestMICPipeline] Play \n");
	gst_element_set_state (m_pipeHandle, GST_STATE_PLAYING);
	return true;
}

/**
 * ================================================================================
 * @fn : Unload
 * @brief : This function unloads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - sets the Gstreamer state to GST_STATE_NULL
 * - removes the pipeline object
 * - stops the main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestMICPipeline::Unload()
{

	XCALLLOGD("[UnitTestMICPipeline] Unload \n");

	gst_element_set_state (m_pipeHandle, GST_STATE_NULL);
	gst_object_unref( GST_OBJECT(m_pipeHandle) );

	return true;
}

/**
 * ================================================================================
 * @fn : gstBusCallbackHandle
 * @brief : call back function for error message handling
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - parses the error information
 * - Handles necessary action as per the error inforamtion
 * @param[in] pBus       : Gstreamer asynchronous message bus subsystem object
 * @param[in] GstMessage :  objects to signal the application of pipeline events
 * @param[in] data       : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
gboolean UnitTestMICPipeline::gstBusCallbackHandle (GstBus * pBus, GstMessage * pMessage, gpointer data)
{
	gboolean ret = true;
	UnitTestMICPipeline *self = reinterpret_cast < UnitTestMICPipeline * >(data);

	//TODO: change to PERI LOG print.
	XCALLLOGD("bus callback msg - element : %s | name : %s",
						GST_OBJECT_NAME (GST_MESSAGE_SRC (pMessage)),
						gst_message_type_get_name (GST_MESSAGE_TYPE (pMessage)));

	if (self->m_pipeHandle == NULL) {	 //(GST_IS_ELEMENT(pPlayerHandle->player) == false))
		XCALLLOGD("[BUS]Error. Player Handle is NULL!!! ");
//	goto done;
	}

	switch (GST_MESSAGE_TYPE (pMessage)) {
		case GST_MESSAGE_STATE_CHANGED: {

				GstState oldState = GST_STATE_NULL;
				GstState newState = GST_STATE_NULL;
				GstState pending = GST_STATE_NULL;

				gst_message_parse_state_changed (pMessage, &oldState, &newState, &pending);

				XCALLLOGD("[BUS] Get STATE Changed message...%s -> %s", gst_element_state_get_name (oldState),
							gst_element_state_get_name (newState));
			}
			break;
		case GST_MESSAGE_EOS: {
				XCALLLOGD("[BUS] GST_MESSAGE_EOS ^0^; ");
			}
			break;
		case GST_MESSAGE_ERROR: {
				GError *pErr = NULL;
				gchar *pDebug = NULL;

				gst_message_parse_error (pMessage, &pErr, &pDebug);
				XCALLLOGD("[BUS] Error : %s", pErr->message);
				g_error_free (pErr);
				g_free (pDebug);
			}
			break;
		default: {
				XCALLLOGD("[BUS] Default Gst Type : %d", GST_MESSAGE_TYPE (pMessage));
			}
			break;
	}

	XCALLLOGD("");
//done:
//  gst_object_unref (pMessage);
  return ret;
}

/**
 * ================================================================================
 * @fn : setSourceInformation
 * @brief : Dummy implementation for pure virtual function
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - stores the IP, port, encoding scheme values in member variables of pipeline
 * @param[in] xcall_debug_input   : Structure contains IP, Port, encoding scheme details
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
#ifdef DEBUG_ENABLED
void UnitTestMICPipeline::setSourceInformation(xcall_debug_input input){
	// NULL method
}
#endif

bool UnitTestMICPipeline::Resume(){
	XCALLLOGI("UnitTestMICPipeline::Resume\n");
	return true;
}

bool UnitTestMICPipeline::Pause(){
	XCALLLOGI("UnitTestMICPipeline::Pause\n");
	return true;
}

} //temxcall


