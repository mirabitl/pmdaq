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
  ~MGROCSlow() { ; }
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
    _jjasic["ON_OFF_PA"]=getON_OFF_PA();
  _jjasic["EN_GBST"]=getEN_GBST();
  _jjasic["ON_OFF_SH_HG"]=getON_OFF_SH_HG();
  _jjasic["ON_OFF_SH_LG"]=getON_OFF_SH_LG();
  _jjasic["ON_OFF_WIDLAR"]=getON_OFF_WIDLAR();
  _jjasic["Valid_SH_HG"]=getValid_SH_HG();
  _jjasic["ON_OFF_4BITS"]=getON_OFF_4BITS();
  _jjasic["EN_OTAQ"]=getEN_OTAQ();
  _jjasic["ON_OFF_OTAQ"]=getON_OFF_OTAQ();
  _jjasic["ON_OFF_DISCRI0"]=getON_OFF_DISCRI0();
  _jjasic["ON_OFF_DISCRI2"]=getON_OFF_DISCRI2();
  _jjasic["ON_OFF_DISCRI1"]=getON_OFF_DISCRI1();
  _jjasic["RS_OR_DISCRI"]=getRS_OR_DISCRI();
  _jjasic["EN_PP_BANDGAP"]=getEN_PP_BANDGAP();
  _jjasic["ON_OFF_BG"]=getON_OFF_BG();
  _jjasic["EN_PP_DAC"]=getEN_PP_DAC();
  _jjasic["ON_OFF_DAC"]=getON_OFF_DAC();
  _jjasic["TRIG2B"]=getTRIG2B();
  _jjasic["TRIG1B"]=getTRIG1B();
  _jjasic["TRIG0B"]=getTRIG0B();
  _jjasic["EN_TRIG_OUT"]=getEN_TRIG_OUT();
  _jjasic["DISC_OR_OR"]=getDISC_OR_OR();
  _jjasic["TRIG_EXT_VALIDATION"]=getTRIG_EXT_VALIDATION();
  _jjasic["RAZ_CHN_INT"]=getRAZ_CHN_INT();
  _jjasic["RAZ_CHN_EXT"]=getRAZ_CHN_EXT();
  _jjasic["SC_ON_5MHZ"]=getSC_ON_5MHZ();
  _jjasic["CK_MUX"]=getCK_MUX();
  _jjasic["SEL_RAZ1"]=getSEL_RAZ1();
  _jjasic["SEL_RAZ0"]=getSEL_RAZ0();
  _jjasic["SEL_ENDREADOUT"]=getSEL_ENDREADOUT();
  _jjasic["SEL_STARTREADOUT"]=getSEL_STARTREADOUT();
  _jjasic["EN_OC_CHIPSATB"]=getEN_OC_CHIPSATB();
  _jjasic["EN_OC_TRANSMITON2B"]=getEN_OC_TRANSMITON2B();
  _jjasic["EN_OC_TRANSMITON1B"]=getEN_OC_TRANSMITON1B();
  _jjasic["EN_OC_DOUT2B"]=getEN_OC_DOUT2B();
  _jjasic["EN_OC_DOUT1B"]=getEN_OC_DOUT1B();
  _jjasic["HEADER"] = getHEADER();
  _jjasic["SW_LG"]=getSW_LG();
  _jjasic["SW_HG"]=getSW_HG();
  _jjasic["BB0"]=getBB0();
  _jjasic["BB1"]=getBB1();
  _jjasic["BB2"]=getBB2();

    json::value ctest;
    json::value m0;
    json::value m1;
    json::value m2;
    json::value pag;
    for (int ch = 0; ch < 64; ch++)
    {
      ctest[ch] = ((int)getCTEST(ch));
      pag[ch] = ((int) getDAC4BITS(ch));
      m0[ch] = ((int)getMASKChannel(0, ch));
      m1[ch] = ((int)getMASKChannel(1, ch));
      m2[ch] = ((int)getMASKChannel(2, ch));
    }
    _jjasic["CTEST"] = ctest;
    _jjasic["MASK0"] = m0;
    _jjasic["MASK1"] = m1;
    _jjasic["MASK2"] = m2;
    _jjasic["DAC4BITS"] = pag;

    return _jjasic;
  }
  void dumpJson()
  {

    std::cout << this->toJson() << std::endl;
  }
  void dumpBinary()
  {
    uint8_t *b = (uint8_t *)_l;
    for (int i = 74; i >= 0; i--)
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
    for (auto it = vasic["DAC4BITS"].as_array().begin(); it != vasic["DAC4BITS"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setDAC4BITS(ch, sid);
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
     setON_OFF_PA(vasic["ON_OFF_PA"].as_integer());
  setEN_GBST(vasic["EN_GBST"].as_integer());
  setON_OFF_SH_HG(vasic["ON_OFF_SH_HG"].as_integer());
  setON_OFF_SH_LG(vasic["ON_OFF_SH_LG"].as_integer());
  setON_OFF_WIDLAR(vasic["ON_OFF_WIDLAR"].as_integer());
  setValid_SH_HG(vasic["Valid_SH_HG"].as_integer());
  setON_OFF_4BITS(vasic["ON_OFF_4BITS"].as_integer());
  setEN_OTAQ(vasic["EN_OTAQ"].as_integer());
  setON_OFF_OTAQ(vasic["ON_OFF_OTAQ"].as_integer());
  setON_OFF_DISCRI0(vasic["ON_OFF_DISCRI0"].as_integer());
  setON_OFF_DISCRI2(vasic["ON_OFF_DISCRI2"].as_integer());
  setON_OFF_DISCRI1(vasic["ON_OFF_DISCRI1"].as_integer());
  setRS_OR_DISCRI(vasic["RS_OR_DISCRI"].as_integer());
  setEN_PP_BANDGAP(vasic["EN_PP_BANDGAP"].as_integer());
  setON_OFF_BG(vasic["ON_OFF_BG"].as_integer());
  setEN_PP_DAC(vasic["EN_PP_DAC"].as_integer());
  setON_OFF_DAC(vasic["ON_OFF_DAC"].as_integer());
  setTRIG2B(vasic["TRIG2B"].as_integer());
  setTRIG1B(vasic["TRIG1B"].as_integer());
  setTRIG0B(vasic["TRIG0B"].as_integer());
  setEN_TRIG_OUT(vasic["EN_TRIG_OUT"].as_integer());
  setDISC_OR_OR(vasic["DISC_OR_OR"].as_integer());
  setTRIG_EXT_VALIDATION(vasic["TRIG_EXT_VALIDATION"].as_integer());
  setRAZ_CHN_INT(vasic["RAZ_CHN_INT"].as_integer());
  setRAZ_CHN_EXT(vasic["RAZ_CHN_EXT"].as_integer());
  setSC_ON_5MHZ(vasic["SC_ON_5MHZ"].as_integer());
  setCK_MUX(vasic["CK_MUX"].as_integer());
  setSEL_RAZ1(vasic["SEL_RAZ1"].as_integer());
  setSEL_RAZ0(vasic["SEL_RAZ0"].as_integer());
  setSEL_ENDREADOUT(vasic["SEL_ENDREADOUT"].as_integer());
  setSEL_STARTREADOUT(vasic["SEL_STARTREADOUT"].as_integer());
  setEN_OC_CHIPSATB(vasic["EN_OC_CHIPSATB"].as_integer());
  setEN_OC_TRANSMITON2B(vasic["EN_OC_TRANSMITON2B"].as_integer());
  setEN_OC_TRANSMITON1B(vasic["EN_OC_TRANSMITON1B"].as_integer());
  setEN_OC_DOUT2B(vasic["EN_OC_DOUT2B"].as_integer());
  setEN_OC_DOUT1B(vasic["EN_OC_DOUT1B"].as_integer());

    setSW_LG(vasic["SW_LG"].as_integer());
    setSW_HG(vasic["SW_HG"].as_integer());

    setBB0(vasic["BB0"].as_integer());
    setBB1(vasic["BB1"].as_integer());
    setBB2(vasic["BB2"].as_integer());

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

    for (int i = 591; i >= 0; i--)
      r->setBitState(591 - i, getBit(i) == 1);
  }

  uint8_t *ucInvertedPtr()
  {

    for (int i = 591; i >= 0; i--)
      setBitState(_li, 591 - i, getBit(_l, i) == 1);

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
