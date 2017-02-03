/*
 * Copyright (c) 2015 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * This file is a part of LogHelperUtility. The latest version may be found
 * in http://mod.lge.com/hub/yunsik.jang/loghelper
 */

#ifndef __TEMXCALL_LOG_H__
#define __TEMXCALL_LOG_H__

#ifdef LOGSCOPE_ENALBE
	#define AUTOSCOPE() AutoScopeLogHelper __autoscope__##__FUNCTION__(__PRETTY_FUNCTION__)
#else
	#define AUTOSCOPE() "// do nothing"
#endif

// default context
#define LOGF(...) LogHelper::get()->log(0, LOGLEVEL_FATAL, __VA_ARGS__)
#define LOGE(...) LogHelper::get()->log(0, LOGLEVEL_ERROR, __VA_ARGS__)
#define LOGW(...) LogHelper::get()->log(0, LOGLEVEL_WARN, __VA_ARGS__)
#define LOGI(...) LogHelper::get()->log(0, LOGLEVEL_INFO, __VA_ARGS__)
#define LOGD(...) LogHelper::get()->log(0, LOGLEVEL_DEBUG, __VA_ARGS__)
#define LOGV(...) LogHelper::get()->log(0, LOGLEVEL_VERBOSE, __VA_ARGS__)

// logging with context
#define LOGF_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_FATAL, __VA_ARGS__)
#define LOGE_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_ERROR, __VA_ARGS__)
#define LOGW_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_WARN, __VA_ARGS__)
#define LOGI_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_INFO, __VA_ARGS__)
#define LOGD_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_DEBUG, __VA_ARGS__)
#define LOGV_C(ctx,...) LogHelper::get()->log(#ctx, LOGLEVEL_VERBOSE, __VA_ARGS__)

#define LOGLEVEL(level) LogHelper::get()->setLogLevel((level));
#define LOGSCOPE(b) LogHelper::get()->setLogScope((b));

enum LogLevel {
	LOGLEVEL_FATAL = 0,
	LOGLEVEL_ERROR,
	LOGLEVEL_WARN,
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG,
	LOGLEVEL_VERBOSE,
	LOGLEVEL_LOGOFF = -1,
};

class AutoScopeLogHelper;
class LogHelper {
public:
	static LogHelper* get();

	virtual ~LogHelper() {
	}
	virtual void log(const char* ctx, LogLevel level, ...) = 0;
	virtual void scope(AutoScopeLogHelper*, bool enter, const char* func) = 0;
	void setLogLevel(LogLevel level) {
		m_logLevel = level;
	}
	void setLogScope(bool enable) {
		m_logScope = enable;
	}

protected:
	explicit LogHelper() :
			m_logLevel(LOGLEVEL_VERBOSE), m_logScope(false) {
	}
	LogLevel m_logLevel;
	bool m_logScope;
};

// Prints log for scope automatically
class AutoScopeLogHelper {
public:
	explicit AutoScopeLogHelper(const char* f) :
			m_function(f) {
		LogHelper::get()->scope(this, true, m_function);
	}
	~AutoScopeLogHelper() {
		LogHelper::get()->scope(this, false, m_function);
	}
	void setId(unsigned long id) {
		m_id = id;
	}
	unsigned long id() const {
		return m_id;
	}

private:
	unsigned long m_id;
	const char* m_function;
};

#endif // __TEMXCALL_LOG_H__
