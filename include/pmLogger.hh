#pragma once

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logPdaq(Logger::getLogger("PMDAQ"));
static LoggerPtr _logPm(Logger::getLogger("PMDAQ_EVB"));
static LoggerPtr _logPmex(Logger::getLogger("PMDAQ_EXAMPLE"));
//static LoggerPtr _logZdaqex(Logger::getLogger("ZDAQEXAMPLE"));
#define PM_DEBUG(a,b) LOG4CXX_DEBUG(a,__FILE__<<"("<<__LINE__<<")"<<b)
#define PM_INFO(a,b) LOG4CXX_INFO(a,__FILE__<<"("<<__LINE__<<")"<<b)
#define PM_WARN(a,b) LOG4CXX_WARN(a,__FILE__<<"("<<__LINE__<<")"<<b)
#define PM_ERROR(a,b) LOG4CXX_ERROR(a,__FILE__<<"("<<__LINE__<<")"<<b)
#define PM_FATAL(a,b) LOG4CXX_FATAL(a,__FILE__<<"("<<__LINE__<<")"<<b)
