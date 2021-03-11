#ifndef _pm_Logger_
#define _pm_Logger_
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logPm(Logger::getLogger("ZDAQ"));
static LoggerPtr _logPmex(Logger::getLogger("ZDAQEXAMPLE"));

#endif
