#include "MBMDCCManager.hh"

#include <unistd.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>

Mbmdcc2Manager::Mbmdcc2Manager() : _mpi(NULL)
{
  ;
}

void Mbmdcc2Manager::initialise()
{
  // Register state

  this->addState("INITIALISED");
  this->addState("CONFIGURED");
  this->addState("RUNNING");

  this->addTransition("INITIALISE", "CREATED", "INITIALISED", std::bind(&Mbmdcc2Manager::fsm_initialise, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", std::bind(&Mbmdcc2Manager::configure, this, std::placeholders::_1));
  this->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", std::bind(&Mbmdcc2Manager::configure, this, std::placeholders::_1));

  this->addTransition("DESTROY", "CONFIGURED", "CREATED", std::bind(&Mbmdcc2Manager::destroy, this, std::placeholders::_1));
  this->addTransition("DESTROY", "INITIALISED", "CREATED", std::bind(&Mbmdcc2Manager::destroy, this, std::placeholders::_1));

  // Commands
  this->addCommand("STATUS", std::bind(&Mbmdcc2Manager::c_status, this, std::placeholders::_1));
  this->addCommand("READREG", std::bind(&Mbmdcc2Manager::c_readreg, this, std::placeholders::_1));
  this->addCommand("WRITEREG", std::bind(&Mbmdcc2Manager::c_writereg, this, std::placeholders::_1));
  this->addCommand("PAUSE", std::bind(&Mbmdcc2Manager::c_pause, this, std::placeholders::_1));
  this->addCommand("RESUME", std::bind(&Mbmdcc2Manager::c_resume, this, std::placeholders::_1));
  this->addCommand("RESET", std::bind(&Mbmdcc2Manager::c_reset, this, std::placeholders::_1));
  this->addCommand("RESYNC", std::bind(&Mbmdcc2Manager::c_resync, this, std::placeholders::_1));
  this->addCommand("SPILLON", std::bind(&Mbmdcc2Manager::c_spillon, this, std::placeholders::_1));
  this->addCommand("SPILLOFF", std::bind(&Mbmdcc2Manager::c_spilloff, this, std::placeholders::_1));
  this->addCommand("CHANNELON", std::bind(&Mbmdcc2Manager::c_channelon, this, std::placeholders::_1));

  this->addCommand("RESETTDC", std::bind(&Mbmdcc2Manager::c_resettdc, this, std::placeholders::_1));
  this->addCommand("RESETFSM", std::bind(&Mbmdcc2Manager::c_resetfsm, this, std::placeholders::_1));
  this->addCommand("CALIBON", std::bind(&Mbmdcc2Manager::c_calibon, this, std::placeholders::_1));
  this->addCommand("CALIBOFF", std::bind(&Mbmdcc2Manager::c_caliboff, this, std::placeholders::_1));
  this->addCommand("RELOADCALIB", std::bind(&Mbmdcc2Manager::c_reloadcalib, this, std::placeholders::_1));
  this->addCommand("SETCALIBCOUNT", std::bind(&Mbmdcc2Manager::c_setcalibcount, this, std::placeholders::_1));
  this->addCommand("SETSPILLREGISTER", std::bind(&Mbmdcc2Manager::c_setspillregister, this, std::placeholders::_1));
  this->addCommand("SETCALIBREGISTER", std::bind(&Mbmdcc2Manager::c_setcalibregister, this, std::placeholders::_1));
  this->addCommand("SETHARDRESET", std::bind(&Mbmdcc2Manager::c_sethardreset, this, std::placeholders::_1));
  this->addCommand("SETTRIGEXT", std::bind(&Mbmdcc2Manager::c_settrigext, this, std::placeholders::_1));
  this->addCommand("SETREG", std::bind(&Mbmdcc2Manager::c_setregister, this, std::placeholders::_1));
  this->addCommand("GETREG", std::bind(&Mbmdcc2Manager::c_getregister, this, std::placeholders::_1));
  this->addCommand("SETEXTERNAL", std::bind(&Mbmdcc2Manager::c_setexternaltrigger, this, std::placeholders::_1));
  this->addCommand("SETSPSSPILL", std::bind(&Mbmdcc2Manager::c_setspsspill, this, std::placeholders::_1));

  // Initialise the transport interface.
  _mpi = NULL;
}
void Mbmdcc2Manager::end()
{
  // Stop listening

  if (_mpi != NULL)
  {
    PMF_INFO(_logMbmdcc2, "=> TERMINATE");

    _mpi->terminate();
    PMF_INFO(_logMbmdcc2, "=> CLOSE");
    _mpi->close();

    PMF_INFO(_logMbmdcc2, "=> DELETE");
    if (_mpi != NULL)
      delete _mpi;
    _mpi = NULL;
  }
}

void Mbmdcc2Manager::fsm_initialise(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "****** CMD: INITIALISING");
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;

  // Need a MBMDCC tag
  if (!utils::isMember(params(), "mbmdcc"))
  {
    PMF_ERROR(_logMbmdcc2, " No mbmdcc tag found ");
    par["status"] = json::value::string(U("Missing mbmdcc tag "));
    Reply(status_codes::OK, par);
    return;
  }
  // Now create the Message handler
  if (_mpi == NULL)
    _mpi = new mbmdcc_2::Interface();
  PMF_INFO(_logMbmdcc2, "MPI: INITIALISING");
  _mpi->initialise();

  web::json::value jMBMDCC = params()["mbmdcc"];
  if (!utils::isMember(jMBMDCC, "network"))
  {
    PMF_ERROR(_logMbmdcc2, " No mbmdcc:network tag found ");
    par["status"] = json::value::string(U("Missing mbmdcc::network tag "));
    Reply(status_codes::OK, par);

    return;
  }
  if (!utils::isMember(jMBMDCC, "address"))
  {
    PMF_ERROR(_logMbmdcc2, " No mbmdcc:address tag found ");
    par["status"] = json::value::string(U("Missing mbmdcc::address tag "));
    Reply(status_codes::OK, par);

    return;
  }
  uint32_t ipboard = utils::convertIP(jMBMDCC["address"].as_string());
  // Scan the network
  std::map<uint32_t, std::string> diflist = utils::scanNetwork(jMBMDCC["network"].as_string());
  // Initialise the network
  std::map<uint32_t, std::string>::iterator idif = diflist.find(ipboard);
  if (idif == diflist.end())
  {
    PMF_ERROR(_logMbmdcc2, " No board found at address " << jMBMDCC["address"].as_string());
    par["status"] = json::value::string(U("No board at given address "));
    Reply(status_codes::OK, par);

    return;
  }
  PMF_INFO(_logMbmdcc2, " New MBMDCC found in db " << std::hex << ipboard << std::dec << " IP address " << idif->second);

  _mpi->addDevice(idif->second);

  PMF_INFO(_logMbmdcc2, " Registration done for " << std::hex << ipboard << std::dec);
  PMF_INFO(_logMbmdcc2, "START Listenning" << std::flush);

  // Listen All Mbmdcc sockets
  _mpi->listen();

  // Mask the trigger and reset
  this->maskTrigger();
  this->resetCounter();
  // Reset Busy state
  PMF_INFO(_logMbmdcc2, "Resetting FSM" << std::flush);
  // this->resetFSM(0x1);
  //::usleep(100000);
  // this->resetFSM(0x0);

  PMF_INFO(_logMbmdcc2, "Set parameters" << std::flush);
  this->parseParameters();

  PMF_INFO(_logMbmdcc2, " Init done  ");
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::parseParameters()
{
  if (utils::isMember(params(), "spillon"))
  {
    this->setSpillOn(params()["spillon"].as_integer());
  }
  if (utils::isMember(params(), "spilloff"))
  {
    this->setSpillOff(params()["spilloff"].as_integer());
  }
  if (utils::isMember(params(), "spillregister"))
  {
    this->setSpillRegister(params()["spillregister"].as_integer());
  }
  if (utils::isMember(params(), "channels"))
  {
    this->setChannels(params()["channels"].as_integer());
  }
  if (utils::isMember(params(), "external"))
  {
    this->setExternalTrigger(params()["external"].as_integer());
  }
  if (utils::isMember(params(), "lemomask"))
  {
    this->setLemoMask(params()["lemomask"].as_integer());
  }
}
void Mbmdcc2Manager::configure(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " CMD: CONFIGURING");

  // Now loop on slowcontrol socket
  this->parseParameters();

  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::destroy(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " CMD: CLOSING");
  PMF_INFO(_logMbmdcc2, "CLOSE called ");

  if (_mpi != NULL)
  {
    delete _mpi;
    _mpi = 0;
  }
  PMF_INFO(_logMbmdcc2, " Data sockets deleted");
  par["status"] = json::value::string(U("done"));
  Reply(status_codes::OK, par);

  // To be done: _mbmdcc->clear();
}

void Mbmdcc2Manager::resyncOn() { this->writeRegister(mbmdcc_2::Message::Register::RESYNC_MASK, 0x1); }
void Mbmdcc2Manager::resyncOff() { this->writeRegister(mbmdcc_2::Message::Register::RESYNC_MASK, 0x0); }

uint32_t Mbmdcc2Manager::version() { return this->readRegister(mbmdcc_2::Message::Register::VERSION); }
uint32_t Mbmdcc2Manager::id() { return this->readRegister(mbmdcc_2::Message::Register::ID); }
uint32_t Mbmdcc2Manager::mask() { return this->readRegister(mbmdcc_2::Message::Register::MASK); }
void Mbmdcc2Manager::maskTrigger() { this->writeRegister(mbmdcc_2::Message::Register::MASK, 0xFFFFFFFF); }
void Mbmdcc2Manager::unmaskTrigger() { this->writeRegister(mbmdcc_2::Message::Register::MASK, 0x0); }
uint32_t Mbmdcc2Manager::spillCount() { return this->readRegister(mbmdcc_2::Message::Register::SPILL_CNT); }
void Mbmdcc2Manager::resetCounter()
{
  this->writeRegister(mbmdcc_2::Message::Register::ACQ_CTRL, 0x1);
  this->writeRegister(mbmdcc_2::Message::Register::ACQ_CTRL, 0x0);
}
uint32_t Mbmdcc2Manager::spillOn() { return this->readRegister(mbmdcc_2::Message::Register::SPILL_ON); }
uint32_t Mbmdcc2Manager::spillOff() { return this->readRegister(mbmdcc_2::Message::Register::SPILL_OFF); }
void Mbmdcc2Manager::setSpillOn(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::SPILL_ON, nc); }
void Mbmdcc2Manager::setSpillOff(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::SPILL_OFF, nc); }
uint32_t Mbmdcc2Manager::Channels() { return this->readRegister(mbmdcc_2::Message::Register::CHANNEL_ENABLE); }
void Mbmdcc2Manager::setChannels(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::CHANNEL_ENABLE, nc); }
void Mbmdcc2Manager::calibOn() { this->writeRegister(mbmdcc_2::Message::Register::CALIB_CTRL, 0x2); }
void Mbmdcc2Manager::calibOff() { this->writeRegister(mbmdcc_2::Message::Register::CALIB_CTRL, 0x0); }
uint32_t Mbmdcc2Manager::calibCount() { return this->readRegister(mbmdcc_2::Message::Register::CALIB_NWIN); }
void Mbmdcc2Manager::setCalibCount(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::CALIB_NWIN, nc); }

void Mbmdcc2Manager::setCalibRegister(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::CALIB_CTRL, nc); }

uint32_t Mbmdcc2Manager::hardReset() { return this->readRegister(mbmdcc_2::Message::Register::RESET_FE); }
void Mbmdcc2Manager::setHardReset(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::RESET_FE, nc); }

void Mbmdcc2Manager::setSpillRegister(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::WIN_CTRL, nc); }
uint32_t Mbmdcc2Manager::spillRegister() { return this->readRegister(mbmdcc_2::Message::Register::WIN_CTRL); }
void Mbmdcc2Manager::useSPSSpill(uint32_t len)
{

  if (len == 0)
  {
    this->writeRegister(mbmdcc_2::Message::Register::SPS_SPILL_CTRL, 0);
    this->writeRegister(mbmdcc_2::Message::Register::SPS_SPILL_DURATION, 0);
  }
  else
  {
    this->writeRegister(mbmdcc_2::Message::Register::SPS_SPILL_CTRL, 1);
    this->writeRegister(mbmdcc_2::Message::Register::SPS_SPILL_DURATION, len);
  }
}
void Mbmdcc2Manager::useTrigExt(bool t)
{
  uint32_t reg = this->spillRegister();
  if (t)
    this->setSpillRegister(reg | 2);
  else
    this->setSpillRegister(reg & ~2);
}

void Mbmdcc2Manager::setTriggerDelay(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::TRG_EXT_DELAY, nc); }
uint32_t Mbmdcc2Manager::triggerDelay() { return this->readRegister(mbmdcc_2::Message::Register::TRG_EXT_DELAY); }
void Mbmdcc2Manager::setTriggerBusy(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::TRG_EXT_LEN, nc); }
uint32_t Mbmdcc2Manager::triggerBusy() { return this->readRegister(mbmdcc_2::Message::Register::TRG_EXT_LEN); }

void Mbmdcc2Manager::setExternalTrigger(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::EN_BUSY_TRG, nc); }
uint32_t Mbmdcc2Manager::externalTrigger() { return this->readRegister(mbmdcc_2::Message::Register::EN_BUSY_TRG); }
void Mbmdcc2Manager::setLemoMask(uint32_t nc) { this->writeRegister(mbmdcc_2::Message::Register::LEMO_MASK, nc); }
uint32_t Mbmdcc2Manager::lemoMask() { return this->readRegister(mbmdcc_2::Message::Register::LEMO_MASK); }
void Mbmdcc2Manager::reloadCalibCount()
{

  this->maskTrigger();
  this->writeRegister(mbmdcc_2::Message::Register::WIN_CTRL, 0x8);
  this->writeRegister(mbmdcc_2::Message::Register::CALIB_CTRL, 0x4);
  // sleep(1);
  // this->writeRegister(0x8,0x0);
  // sleep(1);
  this->unmaskTrigger();
  this->calibOn();
}

void Mbmdcc2Manager::resetFSM(uint32_t b) { this->writeRegister(mbmdcc_2::Message::Register::RESET_FSM, b); }
void Mbmdcc2Manager::resetTDC(uint32_t b) { this->writeRegister(mbmdcc_2::Message::Register::RESET_FE, b); }
uint32_t Mbmdcc2Manager::busyCount(uint32_t b) { return this->readRegister(mbmdcc_2::Message::Register::BUSY_0 + (b & 0xF)); }

uint32_t Mbmdcc2Manager::readRegister(uint32_t adr)
{
  uint32_t rc = 0;
  for (auto x : _mpi->boards())
    rc = x.second->reg()->readRegister(adr);
  return rc;
}

void Mbmdcc2Manager::writeRegister(uint32_t adr, uint32_t val)
{
  for (auto x : _mpi->boards())
    x.second->reg()->writeRegister(adr, val);
}

void Mbmdcc2Manager::c_readreg(http_request m)
{
  auto par = json::value::object();

  PMF_INFO(_logMbmdcc2, "Pulse called ");
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t adr = utils::queryIntValue(m, "adr", 0);

  web::json::value r;
  for (auto x : _mpi->boards())
  {
    uint32_t value = x.second->reg()->readRegister(adr);
    PMF_INFO(_logMbmdcc2, "Read reg " << x.second->ipAddress() << " Address " << adr << " Value " << value);
    r[x.second->ipAddress()] = json::value::number(value);
  }

  par["READREG"] = r;
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_writereg(http_request m)
{
  auto par = json::value::object();
  par["STATUS"] = json::value::string(U("DONE"));

  uint32_t adr = utils::queryIntValue(m, "adr", 0);
  uint32_t val = utils::queryIntValue(m, "val", 0);

  web::json::value r;
  for (auto x : _mpi->boards())
  {
    x.second->reg()->writeRegister(adr, val);
    PMF_INFO(_logMbmdcc2, "Write reg " << x.second->ipAddress() << " Address " << adr << " Value " << val);
    r[x.second->ipAddress()] = json::value::number(val);
  }

  par["WRITEREG"] = r;
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_pause(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Pause called ");

  this->maskTrigger();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_resume(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Resume called ");

  this->unmaskTrigger();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_calibon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Calib On called ");
  this->calibOn();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_caliboff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Calib Off called ");
  this->calibOff();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_reloadcalib(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Calib reload called ");
  this->reloadCalibCount();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_setcalibcount(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Calib count called ");
  uint32_t nc = utils::queryIntValue(m, "nclock", 5000000);
  this->setCalibCount(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["NCLOCK"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_reset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " RESET called ");
  this->resetCounter();
  par["STATUS"] = json::value::string(U("DONE"));
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_resync(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Resync switch called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  if (nc == 0)
    this->resyncOff();
  else
    this->resyncOn();
  par["STATUS"] = json::value::string(U("DONE"));
  par["RESYNC"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_spillon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Spill ON called ");

  uint32_t nc = utils::queryIntValue(m, "nclock", 50);
  this->setSpillOn(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["NCLOCK"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_spilloff(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Spill Off called ");

  uint32_t nc = utils::queryIntValue(m, "nclock", 5000);
  this->setSpillOff(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["NCLOCK"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_resettdc(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Reset TDC called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->resetTDC(nc & 0xFFFF);

  par["STATUS"] = json::value::string(U("DONE"));

  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_resetfsm(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Reset FSM called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->resetFSM(nc & 0xFFFF);

  par["STATUS"] = json::value::string(U("DONE"));

  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_channelon(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " beam on time called ");

  uint32_t nc = utils::queryIntValue(m, "value", 1023);
  this->setChannels(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["NCLOCK"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_sethardreset(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Hard reset called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->setHardReset(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_setspillregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "Spill register called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->setSpillRegister(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_setexternaltrigger(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "Spill register called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->setExternalTrigger(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_setspsspill(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "SPS Spill register called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  this->useSPSSpill(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_setregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "Set register called ");

  uint32_t adr = utils::queryIntValue(m, "address", 2);
  uint32_t val = utils::queryIntValue(m, "value", 0);
  PMF_INFO(_logMbmdcc2, "Set register called with " << adr << " => " << val);

  this->writeRegister(adr, val);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(this->readRegister(adr));
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_getregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "Get register called ");

  uint32_t adr = utils::queryIntValue(m, "address", 2);
  PMF_INFO(_logMbmdcc2, "Get register called with " << adr);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(this->readRegister(adr));
  // std::cout<<response<<std::endl;
  Reply(status_codes::OK, par);
}
void Mbmdcc2Manager::c_setcalibregister(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, "Calib register called ");

  uint32_t nc = utils::queryIntValue(m, "value", 0);
  PMF_INFO(_logMbmdcc2, "Calib register called " << nc);
  this->setCalibRegister(nc);

  par["STATUS"] = json::value::string(U("DONE"));
  par["VALUE"] = json::value::number(nc);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_settrigext(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Trig ext setting called ");

  uint32_t delay = utils::queryIntValue(m, "delay", 20);
  uint32_t busy = utils::queryIntValue(m, "busy", 20);
  this->setTriggerDelay(delay);
  this->setTriggerBusy(busy);

  par["STATUS"] = json::value::string(U("DONE"));
  par["DELAY"] = json::value::number(delay);
  par["BUSY"] = json::value::number(busy);
  Reply(status_codes::OK, par);
}

void Mbmdcc2Manager::c_status(http_request m)
{
  auto par = json::value::object();
  PMF_INFO(_logMbmdcc2, " Status called ");

  web::json::value rc;
  rc["version"] = json::value::number(this->version());
  rc["id"] = json::value::number(this->id());
  rc["mask"] = json::value::number(this->mask());
  rc["hard"] = json::value::number(this->hardReset());
  rc["spill"] = json::value::number(this->spillCount());
  uint32_t chen = this->Channels();
  for (int i = 0; i < 16; i++)
  {
    if ((chen >> i) & 1)
    {
      std::stringstream sb;
      sb << "busy" << i;
      rc[sb.str()] = json::value::number(this->busyCount(i));
    }
  }
  rc["spillon"] = json::value::number(this->spillOn());
  rc["spilloff"] = json::value::number(this->spillOff());
  rc["channels"] = json::value::number(this->Channels());
  rc["calib"] = json::value::number(this->calibCount());
  rc["spillreg"] = json::value::number(this->spillRegister());
  rc["trigdelay"] = json::value::number(this->triggerDelay());
  rc["external"] = json::value::number(this->externalTrigger());
  par["COUNTERS"] = rc;
  par["STATUS"] = json::value::string(U("DONE"));
  mqtt_publish("status", par);
  Reply(status_codes::OK, par);
}

extern "C"
{
  // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.
  handlerPlugin *loadProcessor(void)
  {
    return (new Mbmdcc2Manager);
  }
  // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed
  // to it.  This isn't a very safe function, since there's no
  // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(handlerPlugin *obj)
  {
    delete obj;
  }
}
