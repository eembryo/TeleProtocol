
#ifndef __XCALLSERVICE_LOGGER_H__
#define __XCALLSERVICE_LOGGER_H__

#include <string>
#include <vector>
#include <fstream>

#include <dlt/dlt.h>

class xCallBaseLog {
public:
	virtual void fatalLog(const std::string& str) {}
	virtual void errorLog(const std::string& str) {}
	virtual void warningLog(const std::string& str) {}
	virtual void infoLog(const std::string& str) {}
	virtual void debugLog(const std::string& str) {}
	virtual void verboseLog(const std::string& str) {}

protected:
	xCallBaseLog() {};
	virtual ~xCallBaseLog() {};
};

class xCallDLTLogger : public xCallBaseLog {
public:
	xCallDLTLogger(const char* app_name, const char* app_desc, const char* context_name, const char* context_desc);
	~xCallDLTLogger();
	virtual void fatalLog(const std::string& str);
	virtual void errorLog(const std::string& str);
	virtual void warningLog(const std::string& str);
	virtual void infoLog(const std::string& str);
	virtual void debugLog(const std::string& str);
	virtual void verboseLog(const std::string& str);

private:
	DltContext context_;
};

class xCallLog {
public:
	static void RegisterLogger(xCallBaseLog* logger);
	static void UnregisterLogger(xCallBaseLog* logger);
	static bool IsRegistered();

	static void fatalLog(const char* format, ...);
	static void errorLog(const char* format, ...);
	static void warningLog(const char* format, ...);
	static void infoLog(const char* format, ...);
	static void debugLog(const char* format, ...);
	static void verboseLog(const char* format, ...);

private:
	xCallLog();
	static xCallLog* Instance();
	bool registered;
	std::vector<xCallBaseLog*> loggers_;
};

#ifdef DLT_LOG_ENABLE
	#define XCALLLOGF(format, args...)		xCallLog::fatalLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
	#define XCALLLOGE(format, args...)		xCallLog::errorLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
	#define XCALLLOGW(format, args...)		xCallLog::warningLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
	#define XCALLLOGI(format, args...)		xCallLog::infoLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
	#define XCALLLOGD(format, args...)		xCallLog::debugLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
	#define XCALLLOGV(format, args...)		xCallLog::verboseLog("[%s:%d][%s] " format, __FILENAME__, __LINE__, __func__, ##args)
#else
	#define XCALLLOGF(format, args...)		g_print(format, ##args)
	#define XCALLLOGE(format, args...)		g_print(format, ##args)
	#define XCALLLOGW(format, args...)		g_print(format, ##args)
	#define XCALLLOGI(format, args...)		g_print(format, ##args)
	#define XCALLLOGD(format, args...)		g_print(format, ##args)
	#define XCALLLOGV(format, args...)		g_print(format, ##args)
#endif

#endif // __XCALLSERVICE_LOGGER_H__

