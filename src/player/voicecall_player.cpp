
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

#include "voicecall_player.h"

namespace temxcall {

const char * const VoicecallPlayer::playerName = "VoicecallPlayer";
bool VoicecallPlayer::mIsObjRegistered = VoicecallPlayer::RegisterObject();

/**
 * ================================================================================
 * @fn : VoicecallPlayer
 * @brief : This is constructor function for VoicecallPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
VoicecallPlayer::VoicecallPlayer()
	: _pipeline(new VoicecallPipeline())
{

}

/**
 * ================================================================================
 * @fn : ~VoicecallPlayer
 * @brief : This is destructor function for VoicecallPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
VoicecallPlayer::~VoicecallPlayer()
{

}

/**
 * ================================================================================
 * @fn : Load
 * @brief : This function calls the Load function of pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - calls the Load function of pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool VoicecallPlayer::Load()
{

	XCALLLOGD("VoicecallPlayer::Load start\n");
	return _pipeline->Load();
}

/**
 * ================================================================================
 * @fn : play
 * @brief : This function calls the Play function of pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - calls the Play function of pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool VoicecallPlayer::Play()
{

	XCALLLOGD("VoicecallPlayer::play start\n");
	return _pipeline->Play();
}

/**
 * ================================================================================
 * @fn : Unload
 * @brief : This function calls the Unload function of pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - calls the Unload function of pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : boolean value returns function result to caller
 * ================================================================================
 */
bool VoicecallPlayer::Unload()
{

	XCALLLOGD("VoicecallPlayer::Unload start\n");
	return _pipeline->Unload();

}

bool VoicecallPlayer::Pause()
{
	XCALLLOGI("VoicecallPlayer::Pause start\n");
	return _pipeline->Pause();
}

bool VoicecallPlayer::Resume()
{
	XCALLLOGI("VoicecallPlayer::Resume start\n");
	return _pipeline->Resume();
}

/**
 * ================================================================================
 * @fn : setSourceInformation
 * @brief : Fucntion to provide the IP, port, encoding scheme details to Gstreamer pipeline
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - forwards the data to pipeline instance
 * @param[in] xcall_debug_input   : Structure contains IP, Port, encoding scheme details
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
#ifdef DEBUG_ENABLED
void VoicecallPlayer::setSourceInformation(xcall_debug_input input)
{

	XCALLLOGD("VoicecallPlayer::setSourceInformation pipeline setSourceInformation is called \n");
	_pipeline->setSourceInformation(input);

}
#endif

} //temxcall


