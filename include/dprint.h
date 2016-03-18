// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>

G_BEGIN_DECLS

#ifdef DEBUG
#ifdef ENABLE_LOG_COLOR
#define DPRINT(fmt, args...) g_fprintf(stdout, "\033[37;40mDEBUG: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#define DWARN(fmt, args...) g_fprintf(stdout, "\033[37;45mWARNING: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#define DFUNCTION_START	DPRINT("__function_start__\n")
#else  //else ENABLE_LOG_COLOR
#define DPRINT(fmt, args...) g_fprintf(stdout, "DEBUG: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#define DWARN(fmt, args...) g_fprintf(stdout, "WARNING: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#define DFUNCTION_START	DPRINT("__function_start__\n")
#endif //ENABLE_LOG_COLOR
#else //else DEBUG
#define DPRINT(fmt, args...)
#define DWARN(fmt, args...)
#define DFUNCTION_START
#endif //DEBUG

#ifdef ENABLE_LOG_COLOR
#define DERROR(fmt, args...) g_fprintf(stdout, "\033[37;41mERROR: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#else
#define DERROR(fmt, args...) g_fprintf(stdout, "ERROR: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#endif


G_END_DECLS
