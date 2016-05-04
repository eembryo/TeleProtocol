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

#define DLOG_LEVEL_ERROR 0
#define DLOG_LEVEL_CRITICAL 1
#define DLOG_LEVEL_WARN 2
#define DLOG_LEVEL_INFO 3
#define DLOG_LEVEL_DEBUG 4
#define DLOG_LEVEL_ALL 5

#ifdef ENABLE_LOG_COLOR
#define DERROR(fmt, args...) g_fprintf(stdout, "\033[37;41mERROR: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#else
#define DERROR(fmt, args...) g_fprintf(stdout, "ERROR: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#endif


#if DLOG_LEVEL_INFO <= DEBUG
#ifdef ENABLE_LOG_COLOR
#define DINFO(fmt, args...) g_fprintf(stdout, "\033[37;40mINFO: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#else
#define DINFO(fmt, args...) g_fprintf(stdout, "INFO: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#endif //ENABLE_LOG_COLOR
#else
#define DINFO(fmt, args...)
#endif

#if DLOG_LEVEL_WARN <= DEBUG
#ifdef ENABLE_LOG_COLOR
#define DWARN(fmt, args...) g_fprintf(stdout, "\033[37;45mWARNING: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#else
#define DWARN(fmt, args...) g_fprintf(stdout, "WARNING: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#endif //ENABLE_LOG_COLOR
#else
#define DWARN(fmt, args...)
#endif

#if DLOG_LEVEL_DEBUG <= DEBUG
#ifdef ENABLE_LOG_COLOR
#define DPRINT(fmt, args...) g_fprintf(stdout, "\033[37;40mDEBUG: %s():\033[m " fmt, __PRETTY_FUNCTION__, ##args)
#else
#define DPRINT(fmt, args...) g_fprintf(stdout, "DEBUG: %s(): " fmt, __PRETTY_FUNCTION__, ##args)
#endif //ENABLE_LOG_COLOR
#else
#define DPRINT(fmt, args...)
#endif

#if DLOG_LEVEL_ALL <= DEBUG
#define DFUNCTION_START	DPRINT("__function_start__\n")
#else
#define DFUNCTION_START
#endif


G_END_DECLS
