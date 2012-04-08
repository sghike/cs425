#include "log.hpp"
#include <log4cxx/propertyconfigurator.h>

log4cxx::LoggerPtr g_logger = log4cxx::Logger::getRootLogger();

void configureLogging(const char* logconffile)
{
    if (logconffile) {
        log4cxx::PropertyConfigurator::configure(logconffile);
    }
    else {
        // Set up a simple configuration that logs on the console.
        log4cxx::BasicConfigurator::configure();
	g_logger->setLevel(log4cxx::Level::getInfo());

	log4cxx::AppenderList al= g_logger->getAllAppenders();

	for (size_t i = 0; i < al.size(); ++i) {
	    log4cxx::PatternLayoutPtr layout(
                new log4cxx::PatternLayout("%d{MMM dd HH:mm:ss} %-5p %c (%F:%L): %m%n"));
	    al[i]->setLayout(layout);
	}
    }
}
