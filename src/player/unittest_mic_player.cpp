
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

#include "unittest_mic_player.h"

namespace temxcall {

const char * const UnitTestMICPlayer::playerName = "UnitTest-MIC";
bool UnitTestMICPlayer::mIsObjRegistered = UnitTestMICPlayer::RegisterObject();

/**
 * ================================================================================
 * @fn : UnitTestMICPlayer
 * @brief : This is constructor function for UnitTestMICPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * - Initializes the pipeline
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestMICPlayer::UnitTestMICPlayer()
	: _pipeline(new UnitTestMICPipeline())
{

}

/**
 * ================================================================================
 * @fn : ~UnitTestMICPlayer
 * @brief : This is destructor function for UnitTestMICPlayer class
 * @section function_flow Function flow (Pseudo-code or Decision Table)
 * @section global_variable Global Variables : None
 * @section dependencies Dependencies
 * - None
 * @return : None
 * ================================================================================
 */
UnitTestMICPlayer::~UnitTestMICPlayer()
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
bool UnitTestMICPlayer::Load()
{

	XCALLLOGD("[UnitTestMICPlayer] Load \n");
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
bool UnitTestMICPlayer::Play()
{

	XCALLLOGD("[UnitTestMICPlayer] Play \n");
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
bool UnitTestMICPlayer::Unload()
{

	XCALLLOGD("[UnitTestMICPlayer] Unload \n");
	return _pipeline->Unload();
}

bool UnitTestMICPlayer::Pause()
{
	XCALLLOGI("UnitTestMICPlayer::Pause start\n");
	return _pipeline->Pause();
}

bool UnitTestMICPlayer::Resume()
{
	XCALLLOGI("UnitTestMICPlayer::Resume start\n");
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
void UnitTestMICPlayer::setSourceInformation(xcall_debug_input input){
	this->input = input;
}
#endif

} //temxcall



