
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

#include "unittest_spk_pipeline.h"

namespace temxcall {

/**
 * ================================================================================
 * @fn : UnitTestSPKPipeline
 * @brief : This is constructor function for UnitTestSPKPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestSPKPipeline::UnitTestSPKPipeline()
{
	m_pipeHandle = NULL;
	m_busHandler = NULL;
	m_sigBusAsync =0;

	sink = source = decoder = NULL;
}

/**
 * ================================================================================
 * @fn : ~UnitTestSPKPipeline
 * @brief : This is destructor function for UnitTestSPKPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestSPKPipeline::~UnitTestSPKPipeline()
{

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
bool UnitTestSPKPipeline::Load() {

	bool ret = true;

//	gchar *uri_str = "file:///app/share/BluetoothHMI/wav/bell.wav" ;
	const gchar *uri_str = "/app/share/BluetoothHMI/wav/bell.wav" ;

	XCALLLOGD("[UnitTestSPKPipeline] Load -static pipeline (no use of plug:dmix_48000) \n");

	gst_init(0,0);

	m_pipeHandle = gst_pipeline_new("spkunittest");

	if(m_pipeHandle)
	{
		source = gst_element_factory_make("filesrc" , "source");
		decoder = gst_element_factory_make("wavparse" , "decoder");
		sink = gst_element_factory_make ("alsasink", "sink");

		m_busHandler = gst_pipeline_get_bus(GST_PIPELINE(m_pipeHandle));

		if (m_busHandler) {
			gst_bus_add_signal_watch(m_busHandler);
			m_sigBusAsync = g_signal_connect(G_OBJECT(m_busHandler), "message",
									G_CALLBACK(gstBusCallbackHandle), this);
		}

		g_object_set (G_OBJECT (source), "location", uri_str, NULL);
		g_object_set(G_OBJECT(sink), "device", "plug:dmix_48000" /*"hw:2,0"*/, NULL);

	}

	gst_bin_add_many(GST_BIN(m_pipeHandle), source, decoder, sink, NULL);
	gst_element_link_many(source, decoder, sink, NULL);

	gst_element_set_state (m_pipeHandle, GST_STATE_PAUSED);

	return ret;
}

#if 0
bool UnitTestSPKPipeline::Load()
{

	const gchar *uri_str = "file:///app/share/BluetoothHMI/wav/bell.wav" /*UNITTEST_SPK_URL*/ ;

	XCALLLOGD("[UnitTestSPKPipeline] Load \n");

	gst_init(0,0);

	m_pipeHandle = gst_element_factory_make ("playbin", NULL);

	if (m_pipeHandle) {
		XCALLLOGD("created playbin pipeline \n");

		m_busHandler = gst_pipeline_get_bus(GST_PIPELINE(m_pipeHandle));

		if (m_busHandler) {
			gst_bus_add_signal_watch(m_busHandler);
			m_sigBusAsync = g_signal_connect(G_OBJECT(m_busHandler), "message",
									G_CALLBACK(gstBusCallbackHandle), this);
		}

		XCALLLOGD("URL - %s =n",uri_str);

		g_object_set (G_OBJECT (m_pipeHandle), "uri", uri_str, NULL);

		gst_element_set_state (m_pipeHandle, GST_STATE_READY);
		gst_element_set_state (m_pipeHandle, GST_STATE_PAUSED);
	}

	return true;
}
#endif

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
bool UnitTestSPKPipeline::Play()
{

	XCALLLOGD("[UnitTestSPKPipeline] Play \n");
	gst_element_set_state (m_pipeHandle, GST_STATE_PLAYING);
	return true;
}

/**
 * ================================================================================
 * @fn : Unload
 * @brief : This function unloads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - sets the Gstreamer state to GST_STATE_NULL
 * - removes the pipleine object
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestSPKPipeline::Unload()
{
	XCALLLOGD("[UnitTestSPKPipeline] Unload \n");

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
 * @param[in] pMessage   :  objects to signal the application of pipeline events
 * @param[in] data       : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
gboolean UnitTestSPKPipeline::gstBusCallbackHandle (GstBus * pBus, GstMessage * pMessage, gpointer data)
{
	gboolean ret = true;
	UnitTestSPKPipeline *self = reinterpret_cast < UnitTestSPKPipeline * >(data);

	//TODO: change to PERI LOG print.
	XCALLLOGD("bus callback msg - element : %s | name : %s",
						GST_OBJECT_NAME (GST_MESSAGE_SRC (pMessage)),
						gst_message_type_get_name (GST_MESSAGE_TYPE (pMessage)));

	if (self->m_pipeHandle == NULL) {	 //(GST_IS_ELEMENT(pPlayerHandle->player) == false))
		XCALLLOGD("[BUS]Error. Player Handle is NULL!!! ");
//	  goto done;
	}

	switch (GST_MESSAGE_TYPE (pMessage)) {
		case GST_MESSAGE_STATE_CHANGED:{

				GstState oldState = GST_STATE_NULL;
				GstState newState = GST_STATE_NULL;
				GstState pending = GST_STATE_NULL;

				gst_message_parse_state_changed (pMessage, &oldState, &newState, &pending);

				XCALLLOGD("[BUS] Get STATE Changed message...%s -> %s",
									gst_element_state_get_name (oldState),
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
void UnitTestSPKPipeline::setSourceInformation(xcall_debug_input input){
	// NULL method
}
#endif

bool UnitTestSPKPipeline::Resume(){
	XCALLLOGI("UnitTestSPKPipeline::Resume\n");
	return true;
}

bool UnitTestSPKPipeline::Pause(){
	XCALLLOGI("UnitTestSPKPipeline::Pause\n");
	return true;
}

} //temxcall

