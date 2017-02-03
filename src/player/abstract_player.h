
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



#ifndef __TEMXCALL_ABSTRACT_PLAYER_H__
#define __TEMXCALL_ABSTRACT_PLAYER_H__

#include <glib.h>

#ifdef DEBUG_ENABLED
#include <xcall_debug.h>
#endif
#include <xcallservice_logger.h>

namespace temxcall {

class AbstractPlayer {

	public:
		AbstractPlayer() {};
		~AbstractPlayer() {};
		virtual bool Load() = 0;
		virtual bool Play() = 0;
		virtual bool Unload() = 0;
		virtual bool Pause() = 0;
		virtual bool Resume() = 0;
		#ifdef DEBUG_ENABLED
		virtual void setSourceInformation(xcall_debug_input input) = 0;
		#endif

};

} //temxcall

#endif //__TEMXCALL_ABSTRACT_PLAYER_H__

