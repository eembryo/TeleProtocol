

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

#ifndef __TEMXCALL_XCALL_DEBUG_H__
#define __TEMXCALL_XCALL_DEBUG_H__

#include <string>
using namespace std;

namespace temxcall {

#ifdef DEBUG_ENABLED
	typedef struct Xcall_debug_input{
		string destIP;
		int  destPort1;
//		int  destPort2;	not used 
//		int  destPort3;	not used
		string srcIP;
		int  srcPort1;
		string encScheme;
	} xcall_debug_input;
#endif

}   //temxcall
#endif // __TEMXCALL_XCALL_DEBUG_H__


