
#include "xcallservice_logger.h"

#include <stdarg.h>

xCallDLTLogger::xCallDLTLogger(const char* app_name, const char* app_desc, const char* context_name, const char* context_desc) {
	DLT_REGISTER_APP(app_name, app_desc);
	DLT_REGISTER_CONTEXT(context_, context_name, context_desc);
}

xCallDLTLogger::~xCallDLTLogger() {
	DLT_UNREGISTER_CONTEXT(context_);
	DLT_UNREGISTER_APP();
}

void xCallDLTLogger::fatalLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_FATAL, DLT_STRING(str.c_str()));
}

void xCallDLTLogger::errorLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_ERROR, DLT_STRING(str.c_str()));
}

void xCallDLTLogger::warningLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_WARN, DLT_STRING(str.c_str()));
}

void xCallDLTLogger::infoLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_INFO, DLT_STRING(str.c_str()));
}

void xCallDLTLogger::debugLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_DEBUG, DLT_STRING(str.c_str()));
}

void xCallDLTLogger::verboseLog(const std::string& str) {
	DLT_LOG(context_, DLT_LOG_VERBOSE, DLT_STRING(str.c_str()));
}

xCallLog::xCallLog() :
	registered(false),
	loggers_() {
}

xCallLog* xCallLog::Instance() {
	static xCallLog* instance = nullptr;
	if (instance == nullptr) {
		instance = new xCallLog();
	}
	return instance;
}

void xCallLog::RegisterLogger(xCallBaseLog* logger) {
	Instance()->loggers_.push_back(logger);
	Instance()->registered = true;
}

void xCallLog::UnregisterLogger(xCallBaseLog* logger) {

#if 0
	xCallDLTLogger* dlt_log;
	for (auto& iter : Instance()->loggers_) {
		try {
			if ((dlt_log = dynamic_cast<xCallDLTLogger *>(iter)) != nullptr)
				delete dlt_log;
			else
				;
		} catch (const std::bad_cast&) {
			;
		}
	}
#endif
	Instance()->loggers_.clear();
}

bool xCallLog::IsRegistered() {
	return Instance()->registered;
}

#define COMMON_LOG_OUT(LOG_FUNC) do { \
	char buf[512]; \
	va_list args; \
	va_start(args, format); \
	vsnprintf(buf, 512, format, args); \
	va_end(args); \
	std::string str(buf); \
	for (auto& logger : Instance()->loggers_) \
		logger-> LOG_FUNC (str); \
	} while(0);

void xCallLog::fatalLog(const char* format, ...) {
	COMMON_LOG_OUT(fatalLog);
}

void xCallLog::errorLog(const char* format, ...) {
	COMMON_LOG_OUT(errorLog);
}

void xCallLog::warningLog(const char* format, ...) {
	COMMON_LOG_OUT(warningLog);
}

void xCallLog::infoLog(const char* format, ...) {
	COMMON_LOG_OUT(infoLog);
}

void xCallLog::debugLog(const char* format, ...) {
	COMMON_LOG_OUT(debugLog);
}

void xCallLog::verboseLog(const char* format, ...) {
	COMMON_LOG_OUT(verboseLog);
}


