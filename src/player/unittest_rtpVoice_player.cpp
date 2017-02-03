
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

#include "unittest_rtpVoice_player.h"

namespace temxcall {

const char * const UnitTestRTPPlayer::playerName = "UnitTest-RTP";
bool UnitTestRTPPlayer::mIsObjRegistered = UnitTestRTPPlayer::RegisterObject();

/**
 * ================================================================================
 * @fn : UnitTestRTPPlayer
 * @brief : This is constructor function for UnitTestRTPPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestRTPPlayer::UnitTestRTPPlayer()
	: _pipeline(new UnitTestRTPPipeline())
{

}

/**
 * ================================================================================
 * @fn : ~UnitTestRTPPlayer
 * @brief : This is destructor function for UnitTestRTPPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestRTPPlayer::~UnitTestRTPPlayer()
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
bool UnitTestRTPPlayer::Load()
{

	XCALLLOGD("[UnitTestRTPPlayer] Load \n");
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
bool UnitTestRTPPlayer::Play()
{

	XCALLLOGD("[UnitTestRTPPlayer] Play \n");
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
bool UnitTestRTPPlayer::Unload()
{

	XCALLLOGD("[UnitTestRTPPlayer] Unload \n");
	return _pipeline->Unload();
}

bool UnitTestRTPPlayer::Pause()
{
	XCALLLOGI("UnitTestRTPPlayer::Pause start\n");
	return _pipeline->Pause();
}

bool UnitTestRTPPlayer::Resume()
{
	XCALLLOGI("UnitTestRTPPlayer::Resume start\n");
	return _pipeline->Resume();
}

/**
 * ================================================================================
 * @fn : setSourceInformation
 * @brief : Dummy implementation for pure virtual function
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
void UnitTestRTPPlayer::setSourceInformation(xcall_debug_input input)
{
	//_pipeline->setSourceInformation(input);
}
#endif

} //temxcall



