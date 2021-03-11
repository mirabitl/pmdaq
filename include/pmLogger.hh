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

