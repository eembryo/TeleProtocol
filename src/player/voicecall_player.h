
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

#ifndef __TEMXCALL_VOICECALL_PLAYER_H__
#define __TEMXCALL_VOICECALL_PLAYER_H__

#include <memory>

#include "abstract_player.h"
#include "voicecall_pipeline.h"
#include "PlayerFactory.h"

namespace temxcall {

class VoicecallPlayer : public AbstractPlayer  {

public:
	static const char * const playerName;
	VoicecallPlayer() ;
	~VoicecallPlayer() ;

	static VoicecallPlayer* CreateObject () {
		return new VoicecallPlayer();
	}

	virtual bool Load() ;
	virtual bool Play() ;
	virtual bool Unload() ;
	virtual bool Pause();
	virtual bool Resume();

#ifdef DEBUG_ENABLED
	virtual void setSourceInformation(xcall_debug_input input);
#endif

protected:
	std::shared_ptr<VoicecallPipeline> _pipeline;

private:
	static bool RegisterObject() {
		return (sPlayerBaseFactory::GetInstance().Register(
				VoicecallPlayer::playerName, &VoicecallPlayer::CreateObject));
	}
	static bool mIsObjRegistered;
};

}	//temxcall

#endif //__TEMXCALL_VOICECALL_PLAYER_H__

