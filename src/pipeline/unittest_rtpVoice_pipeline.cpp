

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

#include <string.h>


#include "unittest_rtpVoice_pipeline.h"

namespace temxcall {

/**
 * ================================================================================
 * @fn : UnitTestRTPPipeline
 * @brief : This is constructor function for UnitTestRTPPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestRTPPipeline::UnitTestRTPPipeline()
{

	input.destIP.assign("10.177.223.155");
	input.destPort1=5002;

	input.srcIP.assign("10.177.222.192");
	input.srcPort1=5004;

	XCALLLOGD("[Source] %s | %d , , %s | %d \n",input.destIP.c_str(),
											input.destPort1,
											input.srcIP.c_str(), input.srcPort1);

	autoAudioSink = NULL;
	udpSrc1 = udpSrc2 = udpSrcSink = NULL;
	fileSrc = Mad = audioConvertSend = speexEnc = rtpSpeexPay = udpSink1 = udpSink2 = udpSinkSrc = NULL;
	rtpBin = NULL;
	audioReSample = audioConvert = speexDec = rtpSpeexDepay = pipeline = NULL;

	bus = NULL;
	bus_watch_id =0;

	mainLoop = NULL;

	data = {0,};


}

/**
 * ================================================================================
 * @fn : ~UnitTestRTPPipeline
 * @brief : This is destructor function for UnitTestRTPPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestRTPPipeline::~UnitTestRTPPipeline() {

}

/**
 * ================================================================================
 * @fn : InitReceiverPipeline
 * @brief : This function loads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes necessary elements for receiving & processing audio data
 * - connects the elements appropriately
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestRTPPipeline::InitReceiverPipeline()
{
	XCALLLOGD("UnitTestRTPPipeline::InitReceiverPipeline start\n");

	udpSrc1 = gst_element_factory_make("udpsrc", "udpSrc1");

	udpSrc2 = gst_element_factory_make("udpsrc", "udpSrc2");

	rtpSpeexDepay = gst_element_factory_make("rtpspeexdepay", "rtpSpeexDepay");

	speexDec = gst_element_factory_make("speexdec", "speexDec");

	audioConvert = gst_element_factory_make("audioconvert", "audioConvert");
	audioReSample = gst_element_factory_make("audioresample", "audioReSample");
	autoAudioSink = gst_element_factory_make("autoaudiosink", "autoAudioSink");
	udpSrcSink = gst_element_factory_make("udpsink", "udpSrcSink");

	data.rtpSpeexDepay = rtpSpeexDepay;
	data.rtpBin = rtpBin;
	data.udpSrc1 = udpSrc1;
	data.speexDec = speexDec;
	data.audioConvert = audioConvert;
	data.audioReSample = audioReSample;
	data.autoAudioSink = autoAudioSink;

	if (!udpSrc1 || !rtpBin || !rtpSpeexDepay || !speexDec || !audioConvert || !audioReSample || !autoAudioSink || !udpSrcSink || !udpSrc2) {
		XCALLLOGE("InitReceiverPipeline: One element could not be created. Exiting.\n");
		return false;
	}


	// probe addtion
	GstPad * blockpad = gst_element_get_static_pad (udpSrc1, "src");
	gst_pad_add_probe (blockpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, pad_probe_cb, &data, NULL);

	GstCaps *caps = gst_caps_new_simple ("application/x-rtp",
										 "media", G_TYPE_STRING, "audio",
										 "clock-rate",  G_TYPE_INT, 44100,
										 "encoding-name", G_TYPE_STRING, "SPEEX",
										  NULL);

	XCALLLOGD("InitReceiverPipeline reception port %u\n", this->input.srcPort1);
	g_object_set (G_OBJECT (udpSrc1), "port", this->input.srcPort1, NULL);
	g_object_set(G_OBJECT (udpSrc1), "caps", caps, NULL);
	gst_bin_add(GST_BIN(pipeline), udpSrc1);

	XCALLLOGD("InitReceiverPipeline successfull\n");
	return true;
}

/**
 * ================================================================================
 * @fn : InitSenderPipeline
 * @brief : This function loads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes necessary elements for processing & sending audio data
 * - connects the elements appropriately
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestRTPPipeline::InitSenderPipeline()
{
	XCALLLOGD("UnitTestRTPPipeline::InitSenderPipeline start\n");
	GstPad * pad;
	gchar *name;

	fileSrc = gst_element_factory_make("autoaudiosrc", "autoAudioSrc");

	rtpBin = gst_element_factory_make("rtpbin", "rtpBin");
	Mad = gst_element_factory_make("mad", "Mad");
	audioConvertSend = gst_element_factory_make("audioconvert", "audioConvertSend");

	speexEnc = gst_element_factory_make("speexenc", "speexEnc");

	rtpSpeexPay = gst_element_factory_make("rtpspeexpay", "rtpSpeexPay");
	udpSink1 = gst_element_factory_make("udpsink", "udpSink1");
	udpSink2 = gst_element_factory_make("udpsink", "udpSink2");
	udpSinkSrc = gst_element_factory_make("udpsrc", "udpSinkSrc");


	if (!fileSrc || !rtpBin || !audioConvertSend || !speexEnc || !rtpSpeexPay || !udpSink1 || !Mad || !udpSink2 || !udpSinkSrc) {
		XCALLLOGE("InitSenderPipeline: One element could not be created. Exiting.\n");
		return false;
	}


	gst_bin_add_many(GST_BIN(pipeline), fileSrc, audioConvertSend, speexEnc, rtpSpeexPay, rtpBin, udpSink1, udpSink2,udpSinkSrc, NULL);

	/* link */  // speekDecoder needs to be added on needed
	if (!gst_element_link_many (fileSrc, audioConvertSend, speexEnc, rtpSpeexPay, NULL)) {
		XCALLLOGW("Failed to link elements!");
		return false;
	}

	XCALLLOGD("InitReceiverPipeline sending port %s %u\n",this->input.destIP.c_str(),  this->input.destPort1);
	g_object_set (G_OBJECT (udpSink1), "host", this->input.destIP.c_str(), NULL);
	g_object_set (G_OBJECT (udpSink1), "port", this->input.destPort1, NULL);

	g_signal_connect(rtpBin,"pad-added", G_CALLBACK(on_pad_added2), udpSink1);


	pad = gst_element_get_request_pad (rtpBin, "send_rtp_sink_%u");
	name = gst_pad_get_name (pad);
	XCALLLOGD("A rtp bin sink pad of   %s is created\n", name);

	gst_element_link_pads (rtpSpeexPay, "src", rtpBin, name);
	g_free (name);
	gst_object_unref (GST_OBJECT (pad));

	pad = gst_element_get_request_pad (rtpBin, "send_rtcp_src_%u");
	name = gst_pad_get_name (pad);
	XCALLLOGD("A rtp bin _rtcp_src  %s is created\n", name);

	XCALLLOGD("InitReceiverPipeline sending port %s \n",this->input.destIP.c_str());
	g_object_set (G_OBJECT (udpSink2), "host", this->input.destIP.c_str(), NULL);
//	g_object_set (G_OBJECT (udpSink2), "port", this->input.destPort2, NULL); not used
	g_object_set (G_OBJECT (udpSink2), "sync", false, NULL);
	g_object_set (G_OBJECT (udpSink2), "async", false, NULL);

	gst_element_link_pads (rtpBin, name, udpSink2, "sink");
	g_free (name);
	gst_object_unref (GST_OBJECT (pad));

	pad = gst_element_get_request_pad (rtpBin, "recv_rtcp_sink_%u");
	name = gst_pad_get_name (pad);
	XCALLLOGD("A rtp bin recv_rtcp %s is created\n", name);

//	XCALLLOGD("InitReceiverPipeline sending port %u\n",  this->input.destPort3); not used
//	g_object_set (G_OBJECT (udpSinkSrc), "port", this->input.destPort3, NULL);
	gst_element_link_pads (udpSinkSrc, "src", rtpBin, name);
	g_free (name);
	gst_object_unref (GST_OBJECT (pad));

	XCALLLOGD("InitSenderPipeline successfull\n");

	return true;
}

/**
 * ================================================================================
 * @fn : Load
 * @brief : This function loads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes a main loop
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
bool UnitTestRTPPipeline::Load() {

	XCALLLOGD("UnitTestRTPPipeline::Load start\n");

	gst_init(0, 0);
	mainLoop = g_main_loop_new(NULL, FALSE);

	pipeline = gst_pipeline_new("rtpunittest-pipeline");

	data.loop = mainLoop;
	data.pipeline = pipeline;
	data.thisPtr = this;

	if(!pipeline) {
		XCALLLOGE("load: One element could not be created. Exiting.\n");
		return false;
	}

	if(!InitSenderPipeline()) {
		XCALLLOGE("InitSenderPipeline failed Exiting.\n");
		return false;
	}

	if(!InitReceiverPipeline()) {
		XCALLLOGE("InitReceiverPipeline failed Exiting.\n");
		return false;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));

	bus_watch_id = gst_bus_add_watch(bus, bus_call_back, &data);
	gst_element_set_state(pipeline, GST_STATE_PAUSED);

	XCALLLOGD("UnitTestRTPPipeline::Load end\n");
	return true;
}

/**
 * ================================================================================
 * @fn : Play
 * @brief : This function starts playing the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - sets the pipeline status as GST_STATE_PLAYING
 * - start a main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestRTPPipeline::Play() {

	XCALLLOGD("UnitTestRTPPipeline::Play Start\n");

	GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;

	ret = gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
	XCALLLOGD("Now playing: \n");

	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer");

	g_main_loop_run (mainLoop);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		XCALLLOGE("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (pipeline);
		return false;
	}

	XCALLLOGD("UnitTestRTPPipeline::Play end\n");

	return true;

}

/**
 * ================================================================================
 * @fn : Unload
 * @brief : This function unloads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - set the Gstreamer state to GST_STATE_NULL
 * - removes the pipeline object
 * - stops the main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool UnitTestRTPPipeline::Unload() {

	XCALLLOGD("unload function started \n");

	/* Out of the main loop, clean up nicely */
	gst_element_set_state(pipeline, GST_STATE_NULL);

	gst_object_unref (GST_OBJECT (pipeline));
	g_source_remove (bus_watch_id);
	g_main_loop_unref (mainLoop);
	return true;

}

/**
 * ================================================================================
 * @fn : on_pad_added_1
 * @brief : call back function on dynamic pad addition of gstreamer elements
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - connects two gstreamer elements in dynamic pad added event
 * @param[in] element  : Gstreamer element object
 * @param[in] pad      :  newly added pad details
 * @param[in] userData : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void UnitTestRTPPipeline::on_pad_added1 (GstElement *rtpBin, GstPad *pad, gpointer user_data)
{
	XCALLLOGD("UnitTestRTPPipeline::on_pad_added1 start\n");

	CustomData * data = (CustomData *) user_data;
	gchar *name;
	name = gst_pad_get_name(pad);
	XCALLLOGD("on_pad_added1 new pad %s was created\n", name);

	if(strstr(name, "recv_rtp_src_"))
		{
		GstPad *sinkpad;
		sinkpad = gst_element_get_static_pad (data->thisPtr->rtpSpeexDepay, "sink");
		gst_pad_link (pad, sinkpad);
		XCALLLOGD("on_pad_added1 rtpBin rtpspeexdepay is connected\n");
		gst_object_unref (sinkpad);
		gst_element_sync_state_with_parent(data->thisPtr->rtpSpeexDepay);
		gst_element_sync_state_with_parent(data->thisPtr->rtpBin);
		}
	g_free(name);
	XCALLLOGD("UnitTestRTPPipeline::on_pad_added1 end\n");
}

/**
 * ================================================================================
 * @fn : on_pad_added_2
 * @brief : call back function on dynamic pad addition of gstreamer elements
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - connects two gstreamer elements in dynamic pad added event
 * @param[in] element  : Gstreamer element object
 * @param[in] pad1     :  newly added pad details
 * @param[in] userData : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
void UnitTestRTPPipeline::on_pad_added2 (GstElement *element, GstPad *pad1, gpointer data)
{

	XCALLLOGD("UnitTestRTPPipeline::on_pad_added2 start\n");

	GstElement *udpSink1 = (GstElement *) data;
	gchar *name;

	name = gst_pad_get_name(pad1);
	XCALLLOGD("on_pad_added2  %s  created\n", name);

	/*we can now link the pads of source and sink elements*/

	XCALLLOGD("Dynamic pad created, linking the rtp bin / udpSink\n");
	if(strcmp(name, "send_rtp_src_0")==0) {
		GstPad *sinkpad;
		sinkpad = gst_element_get_static_pad (udpSink1, "sink");
		gst_pad_link (pad1, sinkpad);
		gst_object_unref (sinkpad);
		XCALLLOGD("Aon_pad_added2  %s  linking the rtp bin / udpSink isdone \n", name);
	}
	g_free(name);

	gst_element_sync_state_with_parent(element);
	gst_element_sync_state_with_parent(udpSink1);

	XCALLLOGD("UnitTestRTPPipeline::on_pad_added2 end\n");
}


/**
 * ================================================================================
 * @fn : gstBusCallbackHandle
 * @brief : call back function for error message handling
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - parses the error information
 * - Handles necessary action as per the error inforamtion
 * @param[in] bus     : Gstreamer asynchronous message bus subsystem object
 * @param[in] msg     :  objects to signal the application of pipeline events
 * @param[in] usrData : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
gboolean UnitTestRTPPipeline::bus_call_back (GstBus *bus, GstMessage *msg, gpointer usrData)
{

	XCALLLOGD("UnitTestRTPPipeline::bus_call_back start %d\n", GST_MESSAGE_TYPE (msg));
	CustomData *data = (CustomData *)usrData;

	XCALLLOGD("bus callback msg - element : %s | name : %s",
						GST_OBJECT_NAME (GST_MESSAGE_SRC (msg)),
						gst_message_type_get_name (GST_MESSAGE_TYPE (msg)));

	switch (GST_MESSAGE_TYPE (msg)) {
		case GST_MESSAGE_EOS:
				gst_element_set_state (data->pipeline, GST_STATE_READY);
				g_main_loop_quit (data->loop);
			break;
		case GST_MESSAGE_ERROR: {
				gchar *debug;
				GError *error;
				gst_message_parse_error (msg, &error, &debug);
				g_free (debug);
				XCALLLOGE("Error: %s\n", error->message);
				g_error_free (error);
				gst_element_set_state (data->pipeline, GST_STATE_READY);
					g_main_loop_quit (data->loop);
			}
			break;
		case GST_MESSAGE_STATE_CHANGED: {

				GstState oldState = GST_STATE_NULL;
				GstState newState = GST_STATE_NULL;
				GstState pending = GST_STATE_NULL;

				gst_message_parse_state_changed (msg, &oldState, &newState, &pending);

				XCALLLOGD("[BUS] Get STATE Changed message...%s -> %s",
								gst_element_state_get_name (oldState),
								gst_element_state_get_name (newState));

				gchar *pStateName = NULL;
				pStateName = g_strdup_printf ("%s_%s",
									gst_element_state_get_name (oldState),
									gst_element_state_get_name (newState));

				GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN (data->pipeline),
										 GST_DEBUG_GRAPH_SHOW_ALL, pStateName);

				g_free (pStateName);

			}
			break;
		case GST_MESSAGE_CLOCK_LOST:
				/* Get a new clock */
				gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
				gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
		default:
			break;
	}

	XCALLLOGD("UnitTestRTPPipeline::bus_call_back end\n");

	return true;
}

/**
 * ================================================================================
 * @fn : pad_probe_cb
 * @brief : call back function for on data received in pad
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - parses the error information
 * - Handles necessary action as per the error inforamtion
 * @param[in] pad      : gstreamer pad on which data flow started
 * @param[in] info     : addtional pad information
 * @param[in] userData : call back user data
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : enum value <0-4> GstPadProbeReturn
 * ================================================================================
 */
GstPadProbeReturn UnitTestRTPPipeline::pad_probe_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
	CustomData * data = (CustomData *) user_data;

	GstPad * sinkPad;
	gchar *name;
	XCALLLOGD("UnitTestRTPPipeline::pad_probe_cb info.type=%d,  id = %ld \n ", info->type, info->id);

	data->id = info->id;

	gst_pad_remove_probe (pad, GST_PAD_PROBE_INFO_ID (info));

	gst_bin_add_many(GST_BIN(data->thisPtr->pipeline), data->thisPtr->rtpSpeexDepay, data->thisPtr->speexDec, data->thisPtr->audioConvert, data->thisPtr->audioReSample, data->thisPtr->autoAudioSink, NULL);

	g_signal_connect(data->thisPtr->rtpBin,"pad-added", G_CALLBACK(on_pad_added1), data);
	sinkPad = gst_element_get_request_pad (data->thisPtr->rtpBin, "recv_rtp_sink_%u");
	name = gst_pad_get_name (sinkPad);
	XCALLLOGD("UnitTestRTPPipeline::pad_probe_cb  A rtpBin %s is created\n", name);
	gst_element_link_pads (data->thisPtr->udpSrc1, "src", data->thisPtr->rtpBin, name);

	g_free (name);
	gst_object_unref (GST_OBJECT (sinkPad));


	/* link */  // speekDecoder needs to be added on needed
	if (!gst_element_link_many (data->thisPtr->rtpSpeexDepay, data->thisPtr->speexDec,data->thisPtr->audioConvert, data->thisPtr->audioReSample, data->thisPtr->autoAudioSink, NULL)) {
		XCALLLOGW("Failed to link elements!");
	}

	XCALLLOGD("UnitTestRTPPipeline::pad_probe_cb end");
	return GST_PAD_PROBE_OK;
}


#if 0
/* temp micom dbus msg first -> play voice */
gboolean UnitTestRTPPipeline::gstBusCallbackHandle (GstBus * pBus, GstMessage * pMessage, gpointer data){
  gboolean ret = true;
  UnitTestRTPPipeline *self = reinterpret_cast < UnitTestRTPPipeline * >(data);

  //TODO: change to PERI LOG print.
  XCALLLOGD("bus callback msg - element : %s | name : %s",
						  GST_OBJECT_NAME (GST_MESSAGE_SRC (pMessage)),
						  gst_message_type_get_name (GST_MESSAGE_TYPE (pMessage)));

	if (self->m_pipeHandle == NULL) {
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
		}break;
		case GST_MESSAGE_EOS: {
			XCALLLOGD("[BUS] GST_MESSAGE_EOS ^0^; ");
		}break;
		case GST_MESSAGE_ERROR: {
			GError *pErr = NULL;
			gchar *pDebug = NULL;

			gst_message_parse_error (pMessage, &pErr, &pDebug);
			XCALLLOGD("[BUS] Error : %s", pErr->message);

			g_error_free (pErr);
			g_free (pDebug);
		}break;
		default: {
			XCALLLOGD("[BUS] Default Gst Type : %d", GST_MESSAGE_TYPE (pMessage));
		}break;
	}

	XCALLLOGD("");

  return ret;
}
#endif

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
void UnitTestRTPPipeline::setSourceInformation(xcall_debug_input input){

}
#endif

bool UnitTestRTPPipeline::Resume(){
	XCALLLOGI("UnitTestRTPPipeline::Resume\n");
	return true;
}

bool UnitTestRTPPipeline::Pause(){
	XCALLLOGI("UnitTestRTPPipeline::Pause\n");
	return true;
}

} //temxcall



