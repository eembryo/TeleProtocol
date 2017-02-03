
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

#ifndef __TEMXCALL_PLAYER_FACTORY_H__
#define __TEMXCALL_PLAYER_FACTORY_H__



#include "ObjectFactory.h"
#include "Singleton.h"
#include "abstract_player.h"

namespace temxcall {

	class AbstractPlayer;
	typedef std::tr1::function<AbstractPlayer* ()> funcServiceBlockCreator;

	typedef Singleton<ObjectFactory<AbstractPlayer, std::string, funcServiceBlockCreator> >  sPlayerBaseFactory;

} //tempcall
#endif /* __TEMXCALL_PLAYERFACTORY_H__ */
