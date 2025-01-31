#pragma once
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cpprest/json.h>
#include "stdafx.hh"
using namespace std;
using namespace web;
class MGROCSlow
{
public:
  MGROCSlow()
  {
    memset(_l, 0, 28 * sizeof(uint32_t));
  }
  ~HR2Slow() { ; }
  bool getBit(uint32_t *l, int b) { return (l[b / 32] >> (b % 32)) & 1; }
  void setBit(uint32_t *l, int b) { l[b / 32] |= (1 << (b % 32)); };
  void clearBit(uint32_t *l, int b) { l[b / 32] &= ~(1 << (b % 32)); };
  void setBitState(uint32_t *l, int b, bool t)
  {
    if (t)
      setBit(l, b);
    else
      clearBit(l, b);
  }

  bool getBit(int b) { return (_l[b / 32] >> (b % 32)) & 1; }
  void setBit(int b) { _l[b / 32] |= (1 << (b % 32)); };
  void clearBit(int b) { _l[b / 32] &= ~(1 << (b % 32)); };
  void setBitState(int b, bool t)
  {
    if (t)
      setBit(b);
    else
      clearBit(b);
  }
  uint8_t getByte(int b)
  {
    return (getBit(b) | getBit(b + 1) << 1 | getBit(b + 2) << 2 | getBit(b + 3) << 3 | getBit(b + 4) << 4 | getBit(b + 5) << 5 | getBit(b + 6) << 6 | getBit(b + 7) << 7);
  }
  void setByte(int b, uint8_t val, uint8_t len)
  {
    // unsigned char* p=&_l[b/32];
    for (int i = 0; i < len; i++)
      if ((val >> i) & 1)
        setBit(b + i);
      else
        clearBit(b + i);
  }
  void Print()
  {
    for (int i = 0; i < 28; i++)
      printf("%d %x \n", i, _l[i]);
    /* getchar();
     */
  }

  web::json::value toJson()
  {
    auto _jjasic = web::json::value::object();

    _jjasic["ENABLED"] = 1;
    _jjasic["EN_OTAQ"] = getEN_OTAQ();
    _jjasic["DACSW"] = getDACSW();
    _jjasic["SEL0"] = getSEL0();
    _jjasic["CMDB2FSB2"] = getCMDB2FSB2();
    _jjasic["CMDB0FSB2"] = getCMDB0FSB2();
    _jjasic["ENOCTRANSMITON1B"] = getENOCTRANSMITON1B();
    _jjasic["SEL1"] = getSEL1();
    _jjasic["CMDB2FSB1"] = getCMDB2FSB1();
    _jjasic["OTABGSW"] = getOTABGSW();
    _jjasic["SW50F0"] = getSW50F0();
    _jjasic["ENOCDOUT2B"] = getENOCDOUT2B();
    _jjasic["SW50K0"] = getSW50K0();
    _jjasic["SMALLDAC"] = getSMALLDAC();
    _jjasic["SELENDREADOUT"] = getSELENDREADOUT();
    _jjasic["CMDB3FSB1"] = getCMDB3FSB1();
    _jjasic["PWRONBUFF"] = getPWRONBUFF();
    _jjasic["ENOCDOUT1B"] = getENOCDOUT1B();
    _jjasic["SW50K2"] = getSW50K2();
    _jjasic["CMDB2SS"] = getCMDB2SS();
    _jjasic["TRIG1B"] = getTRIG1B();
    _jjasic["CMDB1SS"] = getCMDB1SS();
    _jjasic["PWRONPA"] = getPWRONPA();
    _jjasic["PWRONSS"] = getPWRONSS();
    _jjasic["RAZCHNINTVAL"] = getRAZCHNINTVAL();
    _jjasic["SW100K1"] = getSW100K1();
    _jjasic["CLKMUX"] = getCLKMUX();
    _jjasic["SW50F1"] = getSW50F1();
    _jjasic["PWRONFSB0"] = getPWRONFSB0();
    _jjasic["ENOCTRANSMITON2B"] = getENOCTRANSMITON2B();
    _jjasic["ENOCCHIPSATB"] = getENOCCHIPSATB();
    _jjasic["CMDB3SS"] = getCMDB3SS();
    _jjasic["DISCRI1"] = getDISCRI1();
    _jjasic["SW50F2"] = getSW50F2();
    _jjasic["SW100K0"] = getSW100K0();
    _jjasic["CMDB3FSB2"] = getCMDB3FSB2();
    _jjasic["SCON"] = getSCON();
    _jjasic["TRIG2B"] = getTRIG2B();
    _jjasic["SW100K2"] = getSW100K2();
    _jjasic["SW100F1"] = getSW100F1();
    _jjasic["TRIGEXTVAL"] = getTRIGEXTVAL();
    _jjasic["PWRONFSB2"] = getPWRONFSB2();
    _jjasic["RS_OR_DISCRI"] = getRS_OR_DISCRI();
    _jjasic["TRIG0B"] = getTRIG0B();
    _jjasic["PWRONFSB1"] = getPWRONFSB1();
    _jjasic["SW100F2"] = getSW100F2();
    _jjasic["PWRONW"] = getPWRONW();
    _jjasic["RAZCHNEXTVAL"] = getRAZCHNEXTVAL();
    _jjasic["DISCRI2"] = getDISCRI2();
    _jjasic["SELSTARTREADOUT"] = getSELSTARTREADOUT();
    _jjasic["SW50K1"] = getSW50K1();
    _jjasic["CMDB1FSB1"] = getCMDB1FSB1();
    _jjasic["ENTRIGOUT"] = getENTRIGOUT();
    _jjasic["CMDB0SS"] = getCMDB0SS();
    _jjasic["SW100F0"] = getSW100F0();
    _jjasic["CMDB1FSB2"] = getCMDB1FSB2();
    _jjasic["OTAQ_PWRADC"] = getOTAQ_PWRADC();
    _jjasic["CMDB0FSB1"] = getCMDB0FSB1();
    _jjasic["DISCROROR"] = getDISCROROR();
    _jjasic["DISCRI0"] = getDISCRI0();
    _jjasic["B0"] = getB0();
    _jjasic["B1"] = getB1();
    _jjasic["B2"] = getB2();
    _jjasic["HEADER"] = getHEADER();

    json::value ctest;
    json::value m0;
    json::value m1;
    json::value m2;
    json::value pag;
    for (int ch = 0; ch < 64; ch++)
    {
      ctest[ch] = ((int)getCTEST(ch));
      pag[ch] = ((int)getPAGAIN(ch));
      m0[ch] = ((int)getMASKChannel(0, ch));
      m1[ch] = ((int)getMASKChannel(1, ch));
      m2[ch] = ((int)getMASKChannel(2, ch));
    }
    _jjasic["CTEST"] = ctest;
    _jjasic["MASK0"] = m0;
    _jjasic["MASK1"] = m1;
    _jjasic["MASK2"] = m2;
    _jjasic["PAGAIN"] = pag;

    return _jjasic;
  }
  void dumpJson()
  {

    std::cout << this->toJson() << std::endl;
  }
  void dumpBinary()
  {
    uint8_t *b = (uint8_t *)_l;
    for (int i = 108; i >= 0; i--)
      fprintf(stderr, "%.2x", b[i]);
    fprintf(stderr, "\n");
  }

  void loadJson(std::string fname)
  {
    web::json::value output; // JSON read from input file

    try
    {
      // Open the file stream
      std::ifstream f(fname);
      // String stream for holding the JSON file
      std::stringstream strStream;

      // Stream file stream into string stream
      strStream << f.rdbuf();
      f.close(); // Close the filestream

      // Parse the string stream into a JSON object
      output = web::json::value::parse(strStream);
    }
    catch (web::json::json_exception excep)
    {
      //_jasic=web::json::value::null();
      // throw web::json::json_exception("Error Parsing JSON file " + jsonFileName);
    }
    //_jasic=output;
    return;
  }

  void setFromJson(web::json::value vasic)
  {

    uint8_t ch = 0;
    for (auto it = vasic["PAGAIN"].as_array().begin(); it != vasic["PAGAIN"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setPAGAIN(ch, sid);
      ch++;
    }
    ch = 0;
    for (auto it = vasic["CTEST"].as_array().begin(); it != vasic["CTEST"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setCTEST(ch, sid);
      ch++;
    }
    ch = 0;
    for (auto it = vasic["MASK0"].as_array().begin(); it != vasic["MASK0"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMASKChannel(0, ch, sid);
      ch++;
    }
    ch = 0;
    for (auto it = vasic["MASK1"].as_array().begin(); it != vasic["MASK1"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMASKChannel(1, ch, sid);
      ch++;
    }
    ch = 0;
    for (auto it = vasic["MASK2"].as_array().begin(); it != vasic["MASK2"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMASKChannel(2, ch, sid);
      ch++;
    }

    setEN_OTAQ(vasic["EN_OTAQ"].as_integer());
    setDACSW(vasic["DACSW"].as_integer());
    setSEL0(vasic["SEL0"].as_integer());
    setCMDB2FSB2(vasic["CMDB2FSB2"].as_integer());
    setCMDB0FSB2(vasic["CMDB0FSB2"].as_integer());
    setENOCTRANSMITON1B(vasic["ENOCTRANSMITON1B"].as_integer());
    setSEL1(vasic["SEL1"].as_integer());
    setCMDB2FSB1(vasic["CMDB2FSB1"].as_integer());
    setOTABGSW(vasic["OTABGSW"].as_integer());
    setSW50F0(vasic["SW50F0"].as_integer());
    setENOCDOUT2B(vasic["ENOCDOUT2B"].as_integer());
    setSW50K0(vasic["SW50K0"].as_integer());
    setSMALLDAC(vasic["SMALLDAC"].as_integer());
    setSELENDREADOUT(vasic["SELENDREADOUT"].as_integer());
    setCMDB3FSB1(vasic["CMDB3FSB1"].as_integer());
    setSWSSC(vasic["SWSSC"].as_integer());
    setPWRONBUFF(vasic["PWRONBUFF"].as_integer());
    setENOCDOUT1B(vasic["ENOCDOUT1B"].as_integer());
    setSW50K2(vasic["SW50K2"].as_integer());
    setCMDB2SS(vasic["CMDB2SS"].as_integer());
    setTRIG1B(vasic["TRIG1B"].as_integer());
    setCMDB1SS(vasic["CMDB1SS"].as_integer());
    setPWRONPA(vasic["PWRONPA"].as_integer());
    setPWRONSS(vasic["PWRONSS"].as_integer());
    setRAZCHNINTVAL(vasic["RAZCHNINTVAL"].as_integer());
    setSW100K1(vasic["SW100K1"].as_integer());
    setCLKMUX(vasic["CLKMUX"].as_integer());
    setSW50F1(vasic["SW50F1"].as_integer());
    setPWRONFSB0(vasic["PWRONFSB0"].as_integer());
    setENOCTRANSMITON2B(vasic["ENOCTRANSMITON2B"].as_integer());
    setENOCCHIPSATB(vasic["ENOCCHIPSATB"].as_integer());
    setCMDB3SS(vasic["CMDB3SS"].as_integer());
    setDISCRI1(vasic["DISCRI1"].as_integer());
    setSW50F2(vasic["SW50F2"].as_integer());
    setSW100K0(vasic["SW100K0"].as_integer());
    setCMDB3FSB2(vasic["CMDB3FSB2"].as_integer());
    setSCON(vasic["SCON"].as_integer());
    setTRIG2B(vasic["TRIG2B"].as_integer());
    setSW100K2(vasic["SW100K2"].as_integer());
    setSW100F1(vasic["SW100F1"].as_integer());
    setTRIGEXTVAL(vasic["TRIGEXTVAL"].as_integer());
    setPWRONFSB2(vasic["PWRONFSB2"].as_integer());
    setRS_OR_DISCRI(vasic["RS_OR_DISCRI"].as_integer());
    setTRIG0B(vasic["TRIG0B"].as_integer());
    setPWRONFSB1(vasic["PWRONFSB1"].as_integer());
    setSW100F2(vasic["SW100F2"].as_integer());
    setPWRONW(vasic["PWRONW"].as_integer());
    setRAZCHNEXTVAL(vasic["RAZCHNEXTVAL"].as_integer());
    setDISCRI2(vasic["DISCRI2"].as_integer());
    setSELSTARTREADOUT(vasic["SELSTARTREADOUT"].as_integer());
    setSW50K1(vasic["SW50K1"].as_integer());
    setCMDB1FSB1(vasic["CMDB1FSB1"].as_integer());
    setENTRIGOUT(vasic["ENTRIGOUT"].as_integer());
    setCMDB0SS(vasic["CMDB0SS"].as_integer());
    setSW100F0(vasic["SW100F0"].as_integer());
    setCMDB1FSB2(vasic["CMDB1FSB2"].as_integer());
    setOTAQ_PWRADC(vasic["OTAQ_PWRADC"].as_integer());
    setCMDB0FSB1(vasic["CMDB0FSB1"].as_integer());
    setDISCROROR(vasic["DISCROROR"].as_integer());
    setDISCRI0(vasic["DISCRI0"].as_integer());

    setB0(vasic["B0"].as_integer());
    setB1(vasic["B1"].as_integer());
    setB2(vasic["B2"].as_integer());

    // std::cout<<" ASIC HEADER "<<vasic["HEADER"].as_integer()<<std::endl;
    setHEADER(vasic["HEADER"].as_integer());
    // std::cout<<"GET HEADER "<<(int) getHEADER()<<std::endl;
    if (vasic.as_object().find("ENABLED") != vasic.as_object().end())
      enabled = vasic["ENABLED"].as_integer() == 1;
    else
      enabled = true;
  }

  uint32_t *ptr() { return _l; }
  uint8_t *ucPtr() { return (uint8_t *)_l; }
  // Now real access to register function

   
// Micro-Roc from bit 0
  /**
   * @brief CTEST status for one channel
   * 
   * @param b channel number (from 0)
   * @return true 
   * @return false 
   */
  bool getCTEST(int b) { return getBit(b % 64); }
  /**
   * @brief Set the CTEST status
   * 
   * @param b channel number
   * @param on value 
   */
  void setCTEST(int b, bool on) { setBitState(b % 64, on); }

  /**
   * @brief Get on/off_pa for pp (+ pwr_on_a) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_PA() { return getBit(64); }
  /**
   * @brief Set on/off_pa for pp (+ pwr_on_a) 
   * 
   * @param on Value
   */
  void setON_OFF_PA(bool on) { setBitState(64, on); }

  /**
   * @brief Get en_gbst (enable gain boost) 
   * 
   * @return true 
   * @return false 
   */
  bool getEN_GBST() { return getBit(65); }
  /**
   * @brief Set en_gbst (enable gain boost) 
   * 
   * @param on Value
   */
  void setEN_GBST(bool on) { setBitState(65, on); }

 /**
   * @brief Get On/off sh_hg for power pulsing (+pwr_on_a)
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_SH_HG() { return getBit(66); }
  /**
   * @brief Set On/off sh_hg for power pulsing (+pwr_on_a)
   * 
   * @param on Value
   */
  void setON_OFF_SH_HG(bool on) { setBitState(66, on); }

  /**
   * @brief Get On/off sh_lg (depends on ON/FF sh_hg)
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_SH_LG() { return getBit(67); }
  /**
   * @brief Set On/off sh_lg (depends on ON/FF sh_hg)
   * 
   * @param on Value
   */
  void setON_OFF_SH_LG(bool on) { setBitState(67, on); }

  /**
   * @brief Get SW_LG
   * 
   * @return uint8_t 2 bit value
   */
  uint8_t getSW_LG()
  {
    uint16_t r = 0;
    for (int i = 0; i < 2; i++)
      if (getBit(69 - i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief Set SW_LG
   * 
   * @param val 2 bit value
   */
  void setSW_LG(uint8_t val)
  {
    uint16_t r = val & 0x3;
    for (int i = 0; i < 2; i++)
      setBitState(69-i, r & (1 << i));
  }

  /**
   * @brief Get On/off widlar for pp (+pwr_on_adc) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_WIDLAR() { return getBit(70); }
  /**
   * @brief Set On/off widlar for pp (+pwr_on_adc) 
   * 
   * @param on Value to set
   */
  void setON_OFF_WIDLAR(bool on) { setBitState(70, on); }

  /**
   * @brief Get valid_sh_hg (=q of the FF) 0 (off) And valid_sh_lg for READ register (qb of the FF)
   * 
   * @return true 
   * @return false 
   */
  bool getValid_SH_HG() { return getBit(71); }
  /**
   * @brief Set valid_sh_hg 
   * 
   * @param on 
   */
  void setValid_SH_HG(bool on) { setBitState(71, on); }

  /**
   * @brief Get SW_HG
   * 
   * @return uint8_t 2 bit value
   */
  uint8_t getSW_HG()
  {
    uint16_t r = 0;
    for (int i = 0; i < 2; i++)
      if (getBit(73 - i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief Set SW_HG
   * 
   * @param val 2 bit value
   */
  void setSW_HG(uint8_t val)
  {
    uint16_t r = val & 0x3;
    for (int i = 0; i < 2; i++)
      setBitState(73-i, r & (1 << i));
  }

  /**
   * @brief Get the DAC 4bits for one channel
   * 
   * @param channel number
   * @return uint8_t DAC4bits value
   */
  uint8_t getDAC4BITS(uint8_t channel)
  {
    uint16_t r = 0;
    for (int i = 0; i < 4; i++)
      if (getBit(74 +channel*4+i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief Set the DAC 4bits for one channel
   * 
   * @param channel number
   * @param val value (<16) to be set
   */
  void setDAC4BITS(uint8_t channel,uint8_t val)
  {
    uint16_t r = val & 0xF;
    for (int i = 0; i < 4; i++)
      setBitState(74 + channel*4+i, r & (1 << i));
  }

  /**
   * @brief Get On/off dac_4bit for pp (+pwr_on_a) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_4BITS() { return getBit(330); }
  /**
   * @brief Set On/off dac_4bit for pp (+pwr_on_a) 
   * 
   * @param on Value to be set
   */
  void setON_OFF_4BITS(bool on) { setBitState(330, on); }

  /**
   * @brief Get en_otaq 
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OTAQ() { return getBit(331); }
  /**
   * @brief Set en_otaq 
   * 
   * @param on Value
   */
  void setEN_OTAQ(bool on) { setBitState(331, on); }

  /**
   * @brief Get On/off otaQ for pp (+pwr_on_adc) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_OTAQ() { return getBit(332); }
  /**
   * @brief Set On/off otaQ for pp (+pwr_on_adc) 
   * 
   * @param on Value
   */
  void setON_OFF_OTAQ(bool on) { setBitState(332, on); }

  /**
   * @brief Get on/off discri0 for pp (+pwr_on_a) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_DISCRI0() { return getBit(333); }
  /**
   * @brief Set on/off discri0 for pp (+pwr_on_a) 
   * 
   * @param on Value
   */
  void setON_OFF_DISCRI0(bool on) { setBitState(333, on); }

  /**
   * @brief Get on/off discri2 for pp (+pwr_on_a) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_DISCRI2() { return getBit(334); }
  /**
   * @brief Set on/off discri2 for pp (+pwr_on_a) 
   * 
   * @param on Value
   */
  void setON_OFF_DISCRI2(bool on) { setBitState(334, on); }

  /**
   * @brief Get on/off discri1 for pp (+pwr_on_a) 
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_DISCRI1() { return getBit(335); }
  /**
   * @brief Set on/off discri1 for pp (+pwr_on_a) 
   * 
   * @param on Value
   */
  void setON_OFF_DISCRI0(bool on) { setBitState(335, on); }

  /**
   * @brief Get q2_sc_bias=d_mask=rs_or_discri 
   * 
   * @return true 
   * @return false 
   */
  bool getRS_OR_DISCRI() { return getBit(336); }
  /**
   * @brief Set q2_sc_bias=d_mask=rs_or_discri 
   * 
   * @param on Value
   */
  void setRS_OR_DISCRI(bool on) { setBitState(336, on); }

  /**
   * @brief Get 64bit uint representing the mask value for one level
   * 
   * @param level 0/1/2
   * @return uint64_t  The mask
   */
  uint64_t getMASK(uint8_t level)
  {
    uint64_t mask = 0;
    for (int i = 0; i < 64; i++)
    {
      if (getBit(618 + i * 3 + level) == 1)
        mask |= (1 << i);
    }
    return mask;
  }
  /**
   * @brief Set in one go the mask for 64 channel
   * 
   * @param level 0/1/2
   * @param mask 64 bits mask pattern
   */
  void setMASK(uint8_t level, uint64_t mask)
  {
    for (int i = 0; i < 64; i++)
    {
      setBitState(337 + i * 3 + level, ((mask >> i) & 1) != 0);
    }
  }
  /**
   * @brief Get the mask bit one one channel
   * 
   * @param level Threshold level 0/1/2
   * @param ch channel
   * @return true 
   * @return false 
   */
  bool getMASKChannel(uint8_t level, int ch)
  {
    return getBit(337 + (ch & 63) * 3 + level);
  }
  /**
   * @brief Set the mask bit one one channel
   * 
   * @param level Threshold level 0/1/2
   * @param ch channel
   * @param on Value
   */
  void setMASKChannel(uint8_t level, int ch, bool on)
  {
    int i = ch & 63;
    setBitState(337 + i * 3 + level, on);
  }

/**
 * @brief Get header value
 * 
 * @return uint8_t 8-bit header
 */
  uint8_t getHEADER() { return getByte(529); }
#undef HEADER_INVERTED
#ifndef HEADER_INVERTED
  /**
   * @brief Set header value
   * 
   * @param val 8-bit asic number
   */
  void setHEADER(uint8_t val) { setByte(529, val, 8); }
#else
  void setHEADER(uint16_t val)
  {
    uint16_t r = val & 0xFF;
    for (int i = 0; i < 8; i++)
      setBitState(529 - i, r & (1 << i));
  }
#endif

  /**
   * @brief Get En_pp_bandgap
   * 
   * @return true 
   * @return false 
   */
  bool getEN_PP_BANDGAP() { return getBit(537); }
  /**
   * @brief Set En_pp_bandgap
   * 
   * @param on Value
   */
  void setEN_PP_BANDGAP(bool on) { setBitState(537, on); }

  /**
   * @brief Get On/off bg
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_BG() { return getBit(538); }
  /**
   * @brief Set On/off bg
   * 
   * @param on Value
   */
  void setON_OFF_BG(bool on) { setBitState(538, on); }

/**
   * @brief Get En_pp_dac
   * 
   * @return true 
   * @return false 
   */
  bool getEN_PP_DAC() { return getBit(539); }
  /**
   * @brief Set En_pp_dac
   * 
   * @param on Value
   */
  void setEN_PP_DAC(bool on) { setBitState(539, on); }

  /**
   * @brief Get On/off dac
   * 
   * @return true 
   * @return false 
   */
  bool getON_OFF_DAC() { return getBit(540); }
  /**
   * @brief Set On/off DAC
   * 
   * @param on Value
   */
  void setON_OFF_DAC(bool on) { setBitState(540, on); }

  /**
   * @brief Get BB0 value
   * 
   * @return uint16_t BB0
   */
  uint16_t getBB0()
  {
    uint16_t r = 0;
    for (int i = 0; i < 10; i++)
      if (getBit(541 + i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief set BB0 value
   * 
   * @param val BB0 to be set
   */
  void setBB0(uint16_t val)
  {
    uint16_t r = val & 0x3FF;
    for (int i = 0; i < 10; i++)
      setBitState(541 + i, r & (1 << i));
  }

  /**
   * @brief Get BB1 value
   * 
   * @return uint16_t BB1
   */
  uint16_t getBB1()
  {
    uint16_t r = 0;
    for (int i = 0; i < 10; i++)
      if (getBit(551 + i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief set BB1 value
   * 
   * @param val BB1 to be set
   */
  void setBB1(uint16_t val)
  {
    uint16_t r = val & 0x3FF;
    for (int i = 0; i < 10; i++)
      setBitState(551 + i, r & (1 << i));
  }

  /**
   * @brief Get BB2 value
   * 
   * @return uint16_t BB2
   */
  uint16_t getBB2()
  {
    uint16_t r = 0;
    for (int i = 0; i < 10; i++)
      if (getBit(561 + i))
        r |= (1 << i);
    return r;
  }
  /**
   * @brief set BB2 value
   * 
   * @param val BB2 to be set
   */
  void setBB2(uint16_t val)
  {
    uint16_t r = val & 0x3FF;
    for (int i = 0; i < 10; i++)
      setBitState(561 + i, r & (1 << i));
  }

  /**
   * @brief Get trig2b (nor64_2)
   * 
   * @return true 
   * @return false 
   */
  bool getTRIG2B() { return getBit(571); }
  /**
   * @brief Set trig2b (nor64_2)
   * 
   * @param on value
   */
  void setTRIG2B(bool on) { setBitState(571, on); }

  /**
   * @brief Get trig1b (nor64_1)
   * 
   * @return true 
   * @return false 
   */
  bool getTRIG1B() { return getBit(572); }
  /**
   * @brief Set trig1b (nor64_2)
   * 
   * @param on value
   */
  void setTRIG1B(bool on) { setBitState(572, on); }

  /**
   * @brief Get trig0b (nor64_0)
   * 
   * @return true 
   * @return false 
   */
  bool getTRIG0B() { return getBit(573); }
  /**
   * @brief Set trig0b (nor64_0)
   * 
   * @param on value
   */
  void setTRIG0B(bool on) { setBitState(573, on); }


  /**
   * @brief Get EN_trig_out
   * 
   * @return true 
   * @return false 
   */
  bool getEN_TRIG_OUT() { return getBit(574); }
  /**
   * @brief Set EN_trig_out
   * 
   * @param on value
   */
  void setEN_TRIG_OUT(bool on) { setBitState(574, on); }
  
  
  /**
   * @brief Get disc_or_or
   * 
   * @return true 
   * @return false 
   */
  bool getDISC_OR_OR() { return getBit(575); }
  /**
   * @brief Set disc_or_or
   * 
   * @param on value
   */
  void setDISC_OR_OR(bool on) { setBitState(575, on); }
  
  /**
   * @brief Get trig_ext validation
   * 
   * @return true 
   * @return false 
   */
  bool getTRIG_EXT_VALIDATION() { return getBit(576); }
  /**
   * @brief Set trig_ext validation
   * 
   * @param on value
   */
  void setTRIG_EXT_VALIDATION(bool on) { setBitState(576, on); }
  
  /**
   * @brief Get raz_chn_int validation
   * 
   * @return true 
   * @return false 
   */
  bool getRAZ_CHN_INT() { return getBit(577); }
  /**
   * @brief Set raz_chn_int validation
   * 
   * @param on value
   */
  void setRAZ_CHN_INT(bool on) { setBitState(577, on); }
  
  /**
   * @brief Get raz_chn_ext validation
   * 
   * @return true 
   * @return false 
   */
  bool getRAZ_CHN_EXT() { return getBit(578); }
  /**
   * @brief Set raz_chn_ext validation
   * 
   * @param on value
   */
  void setRAZ_CHN_EXT(bool on) { setBitState(578, on); }

  /**
   * @brief Get Sc_on 5MHz and 40MHz (+lvds_on5 and
lvds_on40)
   * 
   * @return true 
   * @return false 
   */
  bool getSC_ON_5MHZ() { return getBit(579); }
  /**
   * @brief Set Sc_on 5MHz and 40MHz (+lvds_on5 and
lvds_on40)
   * 
   * @param on value
   */
  void setSC_ON_5MHZ(bool on) { setBitState(579, on); }
  
  
  /**
   * @brief Get Ck_mux: choice between sroand sro_pod, ck5
and ck5_pod, ck40 and ck40_pod
   * 
   * @return true 
   * @return false 
   */
  bool getCK_MUX() { return getBit(580); }
  /**
   * @brief Set Ck_mux: choice between sroand sro_pod, ck5
and ck5_pod, ck40 and ck40_pod
   * 
   * @param on value
   */
  void setCK_MUX(bool on) { setBitState(580, on); }
  
  /**
   * @brief Get Sel_raz1 (raz_chn width)
   * 
   * @return true 
   * @return false 
   */
  bool getSEL_RAZ1() { return getBit(581); }
  /**
   * @brief Set Sel_raz1 (raz_chn width)
   * 
   * @param on value
   */
  void setSEL_RAZ1(bool on) { setBitState(581, on); }
  

  /**
   * @brief Get Sel_raz0 (mux raz_chn width)
   * 
   * @return true 
   * @return false 
   */
  bool getSEL_RAZ0() { return getBit(582); }
  /**
   * @brief Set Sel_raz0 (mux raz_chn width)
   * 
   * @param on value
   */
  void setSEL_RAZ0(bool on) { setBitState(582, on); }

  /**
   * @brief Get Sel endreadout 1 or 2
   * 
   * @return true 
   * @return false 
   */
  bool getSEL_ENDREADOUT() { return getBit(585); }
  /**
   * @brief Set Sel endreadout 1 or 2
   * 
   * @param on value
   */
  void setSEL_ENDREADOUT(bool on) { setBitState(585, on); }
  
  /**
   * @brief Get Sel startreadout 1 or 2
   * 
   * @return true 
   * @return false 
   */
  bool getSEL_STARTREADOUT() { return getBit(586); }
  /**
   * @brief Set Sel startreadout 1 or 2
   * 
   * @param on value
   */
  void setSEL_STARTREADOUT(bool on) { setBitState(586, on); }
  
  /**
   * @brief Get EN_OC chipsatb
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OC_CHIPSATB() { return getBit(587); }
  /**
   * @brief Set EN_OC chipsatb
   * 
   * @param on value
   */
  void setEN_OC_CHIPSATB(bool on) { setBitState(587, on); }
  
  
  /**
   * @brief Get EN_OC transmiton2b
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OC_TRANSMITON2B() { return getBit(588); }
  /**
   * @brief Set EN_OC transmiton2b
   * 
   * @param on value
   */
  void setEN_OC_TRANSMITON2B(bool on) { setBitState(588, on); }

  /**
   * @brief Get EN_OC transmiton1b
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OC_TRANSMITON1B() { return getBit(589); }
  /**
   * @brief Set EN_OC transmiton1b
   * 
   * @param on value
   */
  void setEN_OC_TRANSMITON1B(bool on) { setBitState(589, on); }

  /**
   * @brief Get EN_OC dout2b
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OC_DOUT2B() { return getBit(590); }
  /**
   * @brief Set EN_OC dout2b
   * 
   * @param on value
   */
  void setEN_OC_DOUT2B(bool on) { setBitState(590, on); }


  /**
   * @brief Get EN_OC dout1b
   * 
   * @return true 
   * @return false 
   */
  bool getEN_OC_DOUT1B() { return getBit(591); }
  /**
   * @brief Set EN_OC dout1b
   * 
   * @param on value
   */
  void setEN_OC_DOUT1B(bool on) { setBitState(591, on); }

 
  bool isEnabled()
  {
    return enabled;
  }
  void setEnable(bool i) { enabled = i; }
  void invertBits(HR2Slow *r)
  {

    for (int i = 871; i >= 0; i--)
      r->setBitState(871 - i, getBit(i) == 1);
  }

  uint8_t *ucInvertedPtr()
  {

    for (int i = 871; i >= 0; i--)
      setBitState(_li, 871 - i, getBit(_l, i) == 1);

    return (uint8_t *)_li;
  }

  web::json::value getJson() { return this->toJson(); }
  void setJson(web::json::value v) { this->setFromJson(v); }

  void store(std::string fname)
  {
    int fd = ::open(fname.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, S_IRWXU);
    if (fd < 0)
    {

      // LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
      perror("No way to store to file :");
      // std::cout<<" No way to store to file"<<std::endl;
      return;
    }
    int size = 28 * sizeof(uint32_t);
    int ier = ::write(fd, _l, size);
    if (ier != size)
    {
      std::cout << "pb in write \n"
                << ier << std::flush;
      return;
    }
    ::close(fd);
  }
  void load(std::string fname)
  {
    int fd = ::open(fname.c_str(), O_RDONLY);
    if (fd < 0)
    {
      fprintf(stderr, "%s  Cannot open file %s : return code %d \n", __PRETTY_FUNCTION__, fname.c_str(), fd);
    }
    int size_buf = ::read(fd, _l, 28 * sizeof(uint32_t));

    ::close(fd);
    enabled = true;
    // std::cout<<toJson()<<std::endl;
  }

private:
  uint32_t _l[28];
  uint32_t _li[28];
  bool enabled;
  // web::json::value _jasic;
};
