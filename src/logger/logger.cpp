// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "../pch.h"

void Logger::init(const std::string & appName)
{
    openlog(appName.c_str(), LOG_PID | LOG_CONS, LOG_DAEMON);
    
    std::stringstream ss;
    ss << "Initialize logger for application: " << appName; 

    Logger::info(ss.str(), "Logger::init()");
}

void Logger::info(const std::string & message, const std::string & methodName)
{
    syslog(LOG_INFO, "%s: %s", methodName.c_str(), message.c_str());
}

void Logger::warning(const std::string & message, const std::string & methodName)
{
    syslog(LOG_WARNING, "%s: %s", methodName.c_str(), message.c_str());
}

void Logger::error(const std::string &message, const std::string &methodName)
{
    syslog(LOG_ERR, "%s: %s", methodName.c_str(), message.c_str());
}

void Logger::errorWithErrno(const std::string &message, const std::string &methodName)
{
    syslog(LOG_ERR, "%s: %s;\n Error: %s", methodName.c_str(), message.c_str(), strerror(errno));
}

void Logger::close()
{
    closelog();
}
