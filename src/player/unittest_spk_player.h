
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

#ifndef __TEMXCALL_UNITTEST_SPK_PLAYER_H__
#define __TEMXCALL_UNITTEST_SPK_PLAYER_H__

#include <memory>
#include <glib.h>


#include "abstract_player.h"
#include "unittest_spk_pipeline.h"
#include "PlayerFactory.h"


namespace temxcall {

class UnitTestSPKPlayer : public AbstractPlayer  {

public:
	UnitTestSPKPlayer() ;
	~UnitTestSPKPlayer() ;

	static UnitTestSPKPlayer* CreateObject (){
		return new UnitTestSPKPlayer();
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
	std::shared_ptr<UnitTestSPKPipeline> _pipeline;

	static const char * const playerName;

private:
	static bool RegisterObject() {
		return (sPlayerBaseFactory::GetInstance().Register(
				UnitTestSPKPlayer::playerName, &UnitTestSPKPlayer::CreateObject));
	}
	static bool mIsObjRegistered;

#ifdef DEBUG_ENABLED
	xcall_debug_input input;
#endif

};

}	//temxcall

#endif //__TEMXCALL_UNITTEST_SPK_PLAYER_H__

