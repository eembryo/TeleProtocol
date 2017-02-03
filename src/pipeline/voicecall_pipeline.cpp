#include "voicecall_pipeline.h"
#include <glib.h>

namespace temxcall {
static inline void print_active_pad (GstElement *input_selector)
{
	GstPad *active_pad = NULL;
	gchar *name;

	g_object_get ( input_selector, "active-pad", &active_pad, NULL);
	name = gst_pad_get_name(active_pad);
	XCALLLOGI("Input-Selector: Active pad = %s", name);
	g_free(name);
}
/**
 * ================================================================================
 * @fn : VoicecallPipeline
 * @brief : This is constructor function for VoicecallPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the class variables
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
VoicecallPipeline::VoicecallPipeline() :
				m_pipeline(NULL), m_rtpBin(NULL), m_caps(NULL),
				m_udpSrc(NULL), m_inputSelector(NULL), m_rtpDepay(NULL), m_audioDec(NULL), m_audioConvert(NULL), m_audioReSample(NULL), m_audioSink(NULL),
				m_sourceEle(NULL), m_audioConvertSend(NULL), m_audioEnc(NULL), m_rtpPay(NULL), m_udpSink(NULL),
				m_bus(NULL), m_bus_watch_id(0)
{
	if (!gst_is_initialized()) gst_init(NULL, NULL);
}

/**
 * ================================================================================
 * @fn : ~VoicecallPipeline
 * @brief : This is destructor function for VoicecallPipeline class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
VoicecallPipeline::~VoicecallPipeline() {
	Unload();
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
bool VoicecallPipeline::InitReceiverPipeline()
{
	XCALLLOGI("VoicecallPipeline::InitReceiverPipeline start");


	if (!this->m_input.encScheme.compare("PCMA")) {
		m_rtpDepay = gst_element_factory_make("rtppcmadepay", "rtpPcmaDepay");
		m_audioDec = gst_element_factory_make("alawdec", "alawDec");
		XCALLLOGD("VoicecallPipeline::InitReceiverPipeline PCMA");
		m_caps = gst_caps_new_simple ("application/x-rtp",
				"media", G_TYPE_STRING, "audio",
				"clock-rate",  G_TYPE_INT, 8000,
				"encoding-name", G_TYPE_STRING, "PCMA",
				NULL);
	} else if (!this->m_input.encScheme.compare("PCMU")) {
		m_rtpDepay = gst_element_factory_make("rtppcmudepay", "rtpPcmuDepay");
		m_audioDec = gst_element_factory_make("mulawdec", "mulawDec");
		XCALLLOGD("VoicecallPipeline::InitReceiverPipeline PCMU");
		m_caps = gst_caps_new_simple ("application/x-rtp",
				"media", G_TYPE_STRING, "audio",
				"clock-rate",  G_TYPE_INT, 8000,
				"encoding-name", G_TYPE_STRING, "PCMU",
				NULL);
	} else /*if (!this->m_input.encScheme.compare("SPEEX"))*/{
		m_rtpDepay = gst_element_factory_make("rtpspeexdepay", "rtpSpeexDepay");
		m_audioDec = gst_element_factory_make("speexdec", "speexDec");
		XCALLLOGD("VoicecallPipeline::InitReceiverPipeline SPEEX");
		m_caps = gst_caps_new_simple ("application/x-rtp",
				"media", G_TYPE_STRING, "audio",
				"clock-rate",  G_TYPE_INT, 44100,
				"encoding-name", G_TYPE_STRING, "SPEEX",
				NULL);
	}

	XCALLLOGD("InitReceiverPipeline reception port %u", this->m_input.srcPort1);
	m_udpSrc = gst_element_factory_make("udpsrc", "udpSrc");
	g_object_set (G_OBJECT (m_udpSrc), "port", this->m_input.srcPort1, NULL);
	g_object_set(G_OBJECT (m_udpSrc), "caps", m_caps, NULL);

	m_inputSelector = gst_element_factory_make("input-selector", "inputSelector");
	g_object_set (G_OBJECT(m_inputSelector), "sync-streams", FALSE, NULL);
	m_audioConvert = gst_element_factory_make("audioconvert", "audioConvert");
	m_audioReSample = gst_element_factory_make("audioresample", "audioReSample");

#if GEELY_HARDWARE
	/**
	 * expected ALSA settings
	 * rate = 8000
	 * channels = 1
	 * format = SND_PCM_FORMAT_S16_LE
	 * period_size = 128		==>		period_time = 16000 (us)
	 * buffer_size = 128 * 4	==>		buffer_time = 16000*4 = 64000 (us)
	 *
	 */
	XCALLLOGD("VoicecallPipeline::alsasink enabled  now is used");
	m_audioSink = gst_element_factory_make("alsasink", "alsaSink");
	if (m_audioSink) {
		g_object_set(G_OBJECT(m_audioSink), "device", "plug:dmix_48000",
				"sync", false,
				NULL);
		g_object_set(G_OBJECT(m_audioSink), "latency-time", 16000, NULL);
		g_object_set(G_OBJECT(m_audioSink), "buffer-time", 64000, NULL);
	}
#else
	XCALLLOGD("VoicecallPipeline::autoaudiosink is used");
	m_audioSink = gst_element_factory_make("autoaudiosink", "autoAudioSink");
	if (m_audioSink) {
		g_object_set(G_OBJECT(m_audioSink),
				"sync", false,
				NULL);
	}
#endif

#if RECEIVE_FILESINK_ENABLED
	const gchar *filepath = "VoiceAudioreceiveddata.wav";
	fileSink = gst_element_factory_make("filesink", "fileSink");
	g_object_set(GST_OBJECT(fileSink), "location", filepath, NULL);
	wavEnc = gst_element_factory_make("wavenc", "wavEnc");
#endif

	if (!m_udpSrc || !m_rtpBin || !m_rtpDepay || !m_audioDec || !m_audioConvert || !m_audioReSample || !m_audioSink) {
		XCALLLOGE("InitReceiverPipeline: One element could not be created. Exiting.\n");
		return false;
	}
	gst_bin_add_many(GST_BIN(m_pipeline), m_udpSrc, m_inputSelector, m_rtpDepay, m_audioDec, m_audioConvert, m_audioReSample, m_audioSink, NULL);
	gst_element_link_many (m_inputSelector, m_rtpDepay, m_audioDec, m_audioConvert, m_audioReSample, m_audioSink, NULL);

	GstPad * sinkPad = gst_element_get_request_pad (m_rtpBin, "recv_rtp_sink_0");
	XCALLLOGD("VoicecallPipeline::InitReceiverPipeline  A rtpBin recv_rtp_sink_0 is created\n");
	gst_element_link_pads (m_udpSrc, "src", m_rtpBin, "recv_rtp_sink_0");
	gst_object_unref (GST_OBJECT (sinkPad));

	XCALLLOGD("InitReceiverPipeline successfull\n");

	return true;
}

/**
 * ================================================================================
 * @fn : InitReceiverPipeline
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
bool VoicecallPipeline::InitSenderPipeline()
{
	XCALLLOGI("VoicecallPipeline::InitSenderPipeline start\n");
	GstPad * pad;
	gchar *name;

#if GEELY_HARDWARE
	XCALLLOGD("VoicecallPipeline::alsasrc is used\n");
	m_sourceEle = gst_element_factory_make("alsasrc", "alsaSrc");
	if (m_sourceEle){
		g_object_set(G_OBJECT(m_sourceEle), "device", "hw:3,0", NULL);
	}
#else
	XCALLLOGD("VoicecallPipeline::autoaudiosrc is used\n");
	m_sourceEle = gst_element_factory_make("autoaudiosrc", "autoAudioSrc");
#endif

	m_audioConvertSend = gst_element_factory_make("audioconvert", "audioConvertSend");

	if (!this->m_input.encScheme.compare("PCMA")) {
		m_audioEnc = gst_element_factory_make("alawenc", "alawEnc");
		m_rtpPay = gst_element_factory_make("rtppcmapay", "rtpPcmaPay");
		XCALLLOGD("VoicecallPipeline::InitSenderPipeline PCMA\n");
	} else if (!this->m_input.encScheme.compare("PCMU")) {
		m_audioEnc = gst_element_factory_make("mulawenc", "mulawEnc");
		m_rtpPay = gst_element_factory_make("rtppcmupay", "rtpPcmuPay");
		XCALLLOGD("VoicecallPipeline::InitSenderPipeline PCMU\n");
	} else /*if (!this->m_input.encScheme.compare("SPEEX"))*/{
		m_audioEnc = gst_element_factory_make("speexenc", "speexEnc");
		m_rtpPay = gst_element_factory_make("rtpspeexpay", "rtpSpeexPay");
		XCALLLOGD("VoicecallPipeline::InitSenderPipeline SPEEX\n");
	}

	m_udpSink = gst_element_factory_make("udpsink", "udpSink");
	// NOTE: if 'async' is set to 'TRUE', udpSink pauses until udpsrc receives a packet.
	// I think that putting a sender and receiver into one pipeline is not a good idea.
	// Normally, multiple streams involved in the same pipeline, have same destination. And
	// the streams are synchronized at the pipeline.
	g_object_set (G_OBJECT (m_udpSink), "async", FALSE, NULL);

	if (!m_sourceEle || !m_rtpBin || !m_audioConvertSend || !m_audioEnc || !m_rtpPay || !m_udpSink) {
		XCALLLOGE("InitSenderPipeline: One element could not be created. Exiting.\n");
		return false;
	}

	gst_bin_add_many(GST_BIN(m_pipeline), m_sourceEle, m_audioConvertSend, m_audioEnc, m_rtpPay, m_rtpBin, m_udpSink, NULL);

	/* link */  // speekDecoder needs to be added on needed
	if (!gst_element_link_many (m_sourceEle, m_audioConvertSend, m_audioEnc, m_rtpPay, NULL)) {
		XCALLLOGW("Failed to link elements!");
		return false;
	}

	XCALLLOGD("InitReceiverPipeline sending port %s %u\n", this->m_input.destIP.c_str(), this->m_input.destPort1);
	g_object_set (G_OBJECT (m_udpSink), "host", this->m_input.destIP.c_str(), NULL);
	g_object_set (G_OBJECT (m_udpSink), "port", this->m_input.destPort1, NULL);

	GstPad *rtp_sink_pad = gst_element_get_request_pad (m_rtpBin, "send_rtp_sink_0");
	if (!rtp_sink_pad) {
		//IMPL: error handling
	}
	XCALLLOGD("A rtp bin sink pad of send_rtp_sink_0 is created\n");
	gst_element_link_pads (m_rtpPay, "src", m_rtpBin, "send_rtp_sink_0");
	gst_object_unref (GST_OBJECT (rtp_sink_pad));

	GstPad *rtp_src_pad = gst_element_get_static_pad (m_rtpBin, "send_rtp_src_0");
	if (!rtp_sink_pad) {
		//IMPL: error handling
	}
	XCALLLOGD("A rtp bin src pad of send_rtp_src_0 is created\n");
	gst_element_link_pads (m_rtpBin, "send_rtp_src_0", m_udpSink, "sink");
	gst_object_unref (GST_OBJECT (rtp_src_pad));

	XCALLLOGI("InitSenderPipeline successfull\n");

	return true;
}

/**
 * ================================================================================
 * @fn : Load
 * @brief : This function loads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initialize a main loop
 * - creates a pipline object
 * - creates the sender pipline elements and receiver pipeline elements
 * - requests for Gstreamer bus
 * - sets the pipeline status as GST_STATE_READY
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool VoicecallPipeline::Load()
{
	XCALLLOGD("VoicecallPipeline::Load start\n");

	m_pipeline = gst_pipeline_new("voicecall-pipeline");
	if (!m_pipeline) {
		XCALLLOGE("load: One element could not be created. Exiting\n");
		return false;
	}
	m_rtpBin = gst_element_factory_make("rtpbin", "rtpBin");
	if (!m_rtpBin) {
		XCALLLOGE("load: RTPBIN element could not be created. Exiting\n");
		return false;
	}
	g_object_set(G_OBJECT(m_rtpBin), "latency", 10, NULL);
	g_object_set(G_OBJECT(m_rtpBin), "autoremove", TRUE, NULL);

	if (!InitSenderPipeline()) {
		XCALLLOGE("InitSenderPipeline failed Exiting\n");
		return false;
	}

	if (!InitReceiverPipeline()) {
		XCALLLOGE("InitReceiverPipeline failed Exiting.\n");
		return false;
	}

	g_signal_connect(m_rtpBin,"pad-added", G_CALLBACK(on_pad_added), this);
	g_signal_connect(m_rtpBin,"pad-removed", G_CALLBACK(on_pad_removed), this);

	m_bus = gst_pipeline_get_bus(GST_PIPELINE (m_pipeline));
	m_bus_watch_id = gst_bus_add_watch(m_bus, bus_call_back, this);
	gst_object_unref (GST_OBJECT (m_bus));

	gst_element_set_state(m_pipeline, GST_STATE_READY);

	XCALLLOGD("VoicecallPipeline::Load end\n");
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
bool VoicecallPipeline::Play()
{
	XCALLLOGD("VoicecallPipeline::Play Start\n");

	GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;

	ret = gst_element_set_state (GST_ELEMENT (m_pipeline), GST_STATE_PLAYING);
	XCALLLOGI("VoicecallPipeline Now playing...\n");

	if (ret == GST_STATE_CHANGE_FAILURE) {
		XCALLLOGE("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (m_pipeline);
		return false;
	}

	XCALLLOGD("VoicecallPipeline::Play end\n");
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(m_pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer-play");

	return true;
}

/**
 * ================================================================================
 * @fn : Unload
 * @brief : This function unloads the Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - sets the Gstreamer state to GST_STATE_NULL
 * - removes the pipleine object
 * - stops the main loop
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool VoicecallPipeline::Unload()
{
	XCALLLOGD("VoicecallPipeline::unload function started \n");

	/* Out of the main loop, clean up nicely */
	g_source_remove (m_bus_watch_id);
	gst_element_set_state(m_pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (m_sourceEle));
	gst_object_unref (GST_OBJECT (m_pipeline));
	gst_caps_unref (m_caps);

	m_dynamicPadPairs.clear();
	m_pipeline = m_rtpBin = NULL;
	
	//CID 6961501 (#1 of 1): Misused comma operator (NO_EFFECT)
	m_udpSrc= m_rtpDepay = m_audioDec = m_audioConvert = m_audioReSample = m_audioSink = NULL;
	m_sourceEle = m_audioConvertSend = m_audioEnc = m_rtpPay = m_udpSink = NULL;
	m_bus = NULL;

	return true;
}

/**
 * ================================================================================
 * @fn : on_pad_added
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
void VoicecallPipeline::on_pad_added (GstElement *rtpBin, GstPad *pad, gpointer userData)
{

	XCALLLOGD("VoicecallPipeline::on_pad_added start");

	VoicecallPipeline *voicecall = (VoicecallPipeline*)userData;
	gchar *name;
	name = gst_pad_get_name(pad);
	XCALLLOGD("VoicecallPipeline::on_pad_added new pad %s was created", name);

	if (g_str_has_prefix(name, "recv_rtp_src_")) {
		GstPad *sinkpad = gst_element_get_request_pad (voicecall->m_inputSelector, "sink_%u");
		if (!sinkpad) XCALLLOGE("%s: cannot request a dynamic pad from input-selector element",__PRETTY_FUNCTION__);
		gst_pad_link (pad, sinkpad);
		voicecall->m_dynamicPadPairs[pad] = sinkpad;
		g_object_set (G_OBJECT (voicecall->m_inputSelector), "active-pad", sinkpad, NULL);
		XCALLLOGI("VoicecallPipeline::on_pad_added new pad %s is connected.", name);
		print_active_pad (voicecall->m_inputSelector);
	}
	g_free(name);

	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(voicecall->m_pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer-padadded");

	XCALLLOGD("VoicecallPipeline::on_pad_added end");
}

void VoicecallPipeline::on_pad_removed (GstElement *element, GstPad *pad, gpointer userData)
{
	XCALLLOGD("%s: start", __PRETTY_FUNCTION__);

	VoicecallPipeline *voicecall = (VoicecallPipeline*)userData;
	bool need_replace = false;

	/* Release input-selector pad which was linked with 'pad' */
	GstPad *old_peer = voicecall->m_dynamicPadPairs[pad];
	if (!old_peer) return;

	// check that old_peer is active pad
	GstPad *active_pad = NULL;
	g_object_get (voicecall->m_inputSelector, "active-pad", &active_pad, NULL);
	if (old_peer == active_pad) need_replace = true;

	gst_element_release_request_pad (voicecall->m_inputSelector, old_peer);
	gst_object_unref(GST_OBJECT (old_peer));

	voicecall->m_dynamicPadPairs.erase(pad);

	if (need_replace) {
		std::map<GstPad*,GstPad*>::iterator it = voicecall->m_dynamicPadPairs.begin();
		if (it != voicecall->m_dynamicPadPairs.end())
			g_object_set (G_OBJECT (voicecall->m_inputSelector), "active-pad", it->second, NULL);
	}

	print_active_pad (voicecall->m_inputSelector);

	XCALLLOGD("%s: end", __PRETTY_FUNCTION__);
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
#include <stdlib.h>
gboolean VoicecallPipeline::bus_call_back (GstBus *bus, GstMessage *msg, gpointer usrData)
{

	//XCALLLOGD("VoicecallPipeline::bus_call_back start %d\n", GST_MESSAGE_TYPE (msg));
	VoicecallPipeline *voicecall = (VoicecallPipeline *)usrData;
	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_EOS:
		gst_element_set_state (voicecall->m_pipeline, GST_STATE_READY);
		//IMPL: do something
		break;
	case GST_MESSAGE_ERROR: {
		gchar *debug;
		GError *error;
		gst_message_parse_error (msg, &error, &debug);
		XCALLLOGI( "[BUS]GST_MESSAGE_ERROR : %s - %d(from %s), %s, %s\n", (error->domain == GST_STREAM_ERROR ? "GST_STREAM_ERROR" : (error->domain ==
																			GST_CORE_ERROR ? "GST_CORE_ERROR" : (error->domain ==
																			GST_RESOURCE_ERROR ? "GST_RESOURCE_ERROR" : (error->domain ==
																			GST_LIBRARY_ERROR ? "GST_LIBRARY_ERROR" : "UNKNOWN")))),
																			error->code,(GST_OBJECT_NAME (GST_MESSAGE_SRC (msg))), error->message, debug );
		g_free (debug);

		// Currently complete Resource domain errors are handled generally
		// TO DO: Need to handle specific errors in future
		if (error->domain == GST_RESOURCE_ERROR) {
			XCALLLOGI("Retying the voice call pipline on resource error\n");
			voicecall->Unload();
			voicecall->Load();
			voicecall->Play();
			g_error_free (error);
			return true;
		}
		g_error_free (error);
		GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(voicecall->m_pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer-buserror");
		//gst_element_set_state (voicecall->pipeline, GST_STATE_READY);
		//IMPL: do something
	}
	break;
	case GST_MESSAGE_CLOCK_LOST:
		/* Get a new clock */
		gst_element_set_state (voicecall->m_pipeline, GST_STATE_PAUSED);
		gst_element_set_state (voicecall->m_pipeline, GST_STATE_PLAYING);
		break;
	default:
		break;
	}

	//XCALLLOGD("VoicecallPipeline::bus_call_back end\n");

	return true;
}

/**
 * ================================================================================
 * @fn : setSourceInformation
 * @brief : Fucntion to provide the IP, port, encoding scheme details to Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - stores the IP, port, encoding scheme values in member variables of pipeline
 * @param[in] xcall_debug_input   : Structure contains IP, Port, encoding scheme details
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
 #ifdef DEBUG_ENABLED
void VoicecallPipeline::setSourceInformation(xcall_debug_input input)
{
	this->m_input = input;
	XCALLLOGD("ip details are source: %s %u & Destination: %s %u", this->m_input.destIP.c_str(), this->m_input.destPort1, this->m_input.srcIP.c_str(), this->m_input.srcPort1);
}
 #endif

bool VoicecallPipeline::Resume(){
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
	XCALLLOGI("VoicecallPipeline::Resume\n");
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(m_pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer-resume");
	return true;
}

bool VoicecallPipeline::Pause(){
	gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
	XCALLLOGI("VoicecallPipeline::Pause\n");
	GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(m_pipeline),GST_DEBUG_GRAPH_SHOW_ALL,"myplayer-pause");
	return true;
}

} //temxcall
