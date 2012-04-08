#include "log.hpp"


// create a static logger, local to the current function, named with
// the function's name
#define INIT_LOCAL_LOGGER() \
    static log4cxx::LoggerPtr _local_logger = g_logger->getLogger(__func__)

// log using the current function's local logger (created by
// INIT_LOCAL_LOGGER)
#define LOGDEBUG(x) LOG4CXX_DEBUG(_local_logger, x)
#define LOGINFO(x) LOG4CXX_INFO(_local_logger, x)



class MyObject {
public:
    void func(const char* s, int a)
    {
        // the local logger will be named "func"
        INIT_LOCAL_LOGGER();

        // log messages in here will have "func" in the log
        // message

        // log at info level
        LOGINFO("s is " << s << " and a=" << a);

        // log at debug level
        LOGDEBUG("done");
    }
};

void foo()
{
    // the local logger will be named "foo"
    INIT_LOCAL_LOGGER();
    LOGDEBUG("asdf");
}

void bar()
{
    INIT_LOCAL_LOGGER();
    LOGDEBUG("zxcv");
}

int main()
{
    // configure the loggin according to the "logconf" file. which can
    // be left NULL to use default configuration
    configureLogging("logconf");

    MyObject o;
    o.func("abc", 123);

    foo();
    bar();
    return 0;
}
