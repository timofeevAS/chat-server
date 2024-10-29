// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger
{
public:
    static void init(const std::string & appName);
    static void info(const std::string & message, const std::string & methodName);
    static void warning(const std::string & message, const std::string & methodName);
    static void error(const std::string & message, const std::string & methodName);
    static void errorWithErrno(const std::string & message, const std::string & methodName);
    static void close();
};

#endif