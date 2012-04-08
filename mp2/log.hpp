#ifndef LOG_HPP
#define LOG_HPP

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/layout.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/propertyconfigurator.h>

void configureLogging(const char* logconffile);

extern log4cxx::LoggerPtr g_logger;

#endif /* LOG_HPP */
