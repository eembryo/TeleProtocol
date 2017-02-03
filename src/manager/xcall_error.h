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
 * Date : 05/12/2016
 *
 * <IHU XCallService> Detailed note is as follows.
 *
 *
 */

#ifndef __TEMXCALL_XCALL_ERROR_H__
#define __TEMXCALL_XCALL_ERROR_H__

#include <cstring>
#include <string>
#include <map>

namespace temxcall {

typedef enum eXcallSvcErr
{
	xcall_Success = 0,	/**< Success */

	//General erros 100
	xcall_Err_TemNotConnected = 100,
	xcall_Err_TemNoResponse,
	xcall_Err_TemServiceDown,
	xcall_Err_OperationInprogress,
	xcall_Err_InternalErr,
	xcall_Err_InvalidParameters,
	//service related errors
	xcall_Err_Icall_Not_available = 150,
	xcall_Err_TemConnectionLost = 151,

	//Audio related errors 200
} eXcallSvcErr_t;

std::map<std::string, eXcallSvcErr_t> TEMIL_ERR_MAPPING = {
	{"No response",				xcall_Err_TemNotConnected},
	{"Service is Down!",		xcall_Err_TemNotConnected},
	{"Method In Progress!",		xcall_Err_OperationInprogress},
	{"InternalError!",			xcall_Err_InternalErr},
	{"Invalid Parameter",		xcall_Err_InvalidParameters},
	{"Invalid Parameters",		xcall_Err_InvalidParameters},
	{"Invalid Parameters!",		xcall_Err_InvalidParameters},
	{"Encoding Error!",			xcall_Err_InternalErr},
	{"Internal Error",			xcall_Err_InternalErr},
	{"Service not available",	xcall_Err_Icall_Not_available},
	{"TEM Connection Lost",		xcall_Err_TemConnectionLost},
};

static int getXcallErrCode (std::string &errStr) {

	std::string s = "GDBus.Error:org.freedesktop.DBus.Error.Failed: ";
	std::string::size_type i = errStr.find(s);
	if (i != std::string::npos)
		errStr.erase(i, s.length());

	return TEMIL_ERR_MAPPING[errStr];
}

} //temxcall

#endif //__TEMXCALL_XCALL_ERROR_H__
