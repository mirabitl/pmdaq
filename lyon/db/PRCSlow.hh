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
#include "stdafx.hh"
#define SLC_BYTES_LENGTH 83
/**
 * 
 * \file PRCSlow.hh
 * \brief  Access to PETIROC2C slow control information
 * \author L.Mirabito
 * \version 1.0
 * */
using namespace std;
/**
   * \brief Class to manipulate slow control parameters of PETIROC2C
   * */
class PRCSlow
{
public:
  ///  Constructor
  PRClow() { memset(_l, 0, 21 * sizeof(uint32_t)); }
  ///  Get one bit status
  bool getBit(int b) { return (_l[b / 32] >> (b % 32)) & 1; }
  /// set one bit
  void setBit(int b) { _l[b / 32] |= (1 << (b % 32)); };
  ///  clear one bit
  void clearBit(int b) { _l[b / 32] &= ~(1 << (b % 32)); };
  ///  Set the bit state
  void setBitState(int b, bool t)
  {
    if (t)
      setBit(b);
    else
      clearBit(b);
  }
  ///  Get one Byte value at b position
  uint8_t getByte(int b)
  {
    return (getBit(b) | getBit(b + 1) << 1 | getBit(b + 2) << 2 | getBit(b + 3) << 3 | getBit(b + 4) << 4 | getBit(b + 5) << 5 | getBit(b + 6) << 6 | getBit(b + 7) << 7);
  }
  ///  Set at most 8 bits of a byte at b postion
  void setByte(int b, uint8_t val, uint8_t len)
  {
    //unsigned char* p=&_l[b/32];
    for (int i = 0; i < len; i++)
      if ((val >> i) & 1)
        setBit(b + i);
      else
        clearBit(b + i);
  }

  ///  Print Slow Control
  void Print()
  {

    std::cout << "EN10bDac =" << getEN10bDac() << std::endl;
    std::cout << "PP10bDac =" << getPP10bDac() << std::endl;
    std::cout << "EN_adc =" << getEN_adc() << std::endl;
    std::cout << "PP_adc =" << getPP_adc() << std::endl;
    std::cout << "sel_starb_ramp_adc_ext =" << getsel_starb_ramp_adc_ext() << std::endl;
    std::cout << "usebcompensation =" << getusebcompensation() << std::endl;
    std::cout << "EN_bias_dac_delay =" << getEN_bias_dac_delay() << std::endl;
    std::cout << "PP_bias_dac_delay =" << getPP_bias_dac_delay() << std::endl;
    std::cout << "EN_bias_ramp_delay =" << getEN_bias_ramp_delay() << std::endl;
    std::cout << "PP_bias_ramp_delay =" << getPP_bias_ramp_delay() << std::endl;
    std::cout << "EN_discri_delay =" << getEN_discri_delay() << std::endl;
    std::cout << "PP_discri_delay =" << getPP_discri_delay() << std::endl;
    std::cout << "EN_temp_sensor =" << getEN_temp_sensor() << std::endl;
    std::cout << "PP_temp_sensor =" << getPP_temp_sensor() << std::endl;
    std::cout << "EN_bias_pa =" << getEN_bias_pa() << std::endl;
    std::cout << "PP_bias_pa =" << getPP_bias_pa() << std::endl;
    std::cout << "EN_bias_discri =" << getEN_bias_discri() << std::endl;
    std::cout << "PP_bias_discri =" << getPP_bias_discri() << std::endl;
    std::cout << "cmd_polarity =" << getcmd_polarity() << std::endl;
    std::cout << "latch =" << getlatch() << std::endl;
    std::cout << "EN_bias_6bit_dac =" << getEN_bias_6bit_dac() << std::endl;
    std::cout << "PP_bias_6bit_dac =" << getPP_bias_6bit_dac() << std::endl;
    std::cout << "EN_bias_tdc =" << getEN_bias_tdc() << std::endl;
    std::cout << "PP_bias_tdc =" << getPP_bias_tdc() << std::endl;
    std::cout << "ON_OFF_input_dac =" << getON_OFF_input_dac() << std::endl;
    std::cout << "EN_bias_charge =" << getEN_bias_charge() << std::endl;
    std::cout << "PP_bias_charge =" << getPP_bias_charge() << std::endl;
    std::cout << "Cf3_100fF =" << getCf3_100fF() << std::endl;
    std::cout << "Cf2_200fF =" << getCf2_200fF() << std::endl;
    std::cout << "Cf1_2p5pF =" << getCf1_2p5pF() << std::endl;
    std::cout << "Cf0_1p25pF =" << getCf0_1p25pF() << std::endl;
    std::cout << "EN_bias_sca =" << getEN_bias_sca() << std::endl;
    std::cout << "PP_bias_sca =" << getPP_bias_sca() << std::endl;
    std::cout << "EN_bias_discri_charge =" << getEN_bias_discri_charge() << std::endl;
    std::cout << "PP_bias_discri_charge =" << getPP_bias_discri_charge() << std::endl;
    std::cout << "EN_bias_discri_adc_time =" << getEN_bias_discri_adc_time() << std::endl;
    std::cout << "PP_bias_discri_adc_time =" << getPP_bias_discri_adc_time() << std::endl;
    std::cout << "EN_bias_discri_adc_charge =" << getEN_bias_discri_adc_charge() << std::endl;
    std::cout << "PP_bias_discri_adc_charge =" << getPP_bias_discri_adc_charge() << std::endl;
    std::cout << "DIS_razchn_int =" << getDIS_razchn_int() << std::endl;
    std::cout << "DIS_razchn_ext =" << getDIS_razchn_ext() << std::endl;
    std::cout << "SEL_80M =" << getSEL_80M() << std::endl;
    std::cout << "EN_80M =" << getEN_80M() << std::endl;
    std::cout << "EN_slow_lvds_rec =" << getEN_slow_lvds_rec() << std::endl;
    std::cout << "PP_slow_lvds_rec =" << getPP_slow_lvds_rec() << std::endl;
    std::cout << "EN_fast_lvds_rec =" << getEN_fast_lvds_rec() << std::endl;
    std::cout << "PP_fast_lvds_rec =" << getPP_fast_lvds_rec() << std::endl;
    std::cout << "EN_transmitter =" << getEN_transmitter() << std::endl;
    std::cout << "PP_transmitter =" << getPP_transmitter() << std::endl;
    std::cout << "ON_OFF_1mA =" << getON_OFF_1mA() << std::endl;
    std::cout << "ON_OFF_2mA =" << getON_OFF_2mA() << std::endl;
    std::cout << "ON_OFF_otaQ =" << getON_OFF_otaQ() << std::endl;
    std::cout << "ON_OFF_ota_mux =" << getON_OFF_ota_mux() << std::endl;
    std::cout << "ON_OFF_ota_probe =" << getON_OFF_ota_probe() << std::endl;
    std::cout << "DIS_trig_mux =" << getDIS_trig_mux() << std::endl;
    std::cout << "EN_NOR32_time =" << getEN_NOR32_time() << std::endl;
    std::cout << "EN_NOR32_charge =" << getEN_NOR32_charge() << std::endl;
    std::cout << "DIS_triggers =" << getDIS_triggers() << std::endl;
    std::cout << "EN_dout_oc =" << getEN_dout_oc() << std::endl;
    std::cout << "EN_transmit =" << getEN_transmit() << std::endl;
    //std::cout << "EN_transmit =" << getEN_transmit() << std::endl;
    std::cout << "DacDelay =" << getDacDelay() << std::endl;
    for (int ch = 0; ch < 32; ch++)
      std::cout << "InputDac[" << ch << "]=" << (int)getInputDac(ch) << std::endl;
    for (int ch = 0; ch < 32; ch++)
      std::cout << "6bDac[" << ch << "]=" << (int)get6bDac(ch) << std::endl;

    for (int ch = 0; ch < 32; ch++)
      std::cout << "MaskDiscriCharge[" << ch << "]=" << (int)getMaskDiscriCharge(ch) << std::endl;
    for (int ch = 0; ch < 32; ch++)
      std::cout << "MaskDiscriTime[" << ch << "]=" << (int)getMaskDiscriTime(ch) << std::endl;
    for (int ch = 0; ch < 32; ch++)
      std::cout << "InputDacCommand[" << ch << "]=" << (int)getInputDacCommand(ch) << std::endl;

    std::cout << "VthDiscriCharge =" << getVthDiscriCharge() << std::endl;
    std::cout << "VthTime =" << getVthTime() << std::endl;

    for (int i = 0; i < 20; i++)
      printf("%d %x \n", i, _l[i]);
  }

  ///  store in JSON SLC values
  void toJson()
  {
    //_jasic.clear();
    web::json::value _kasic;
    _kasic["EN10bDac"] = getEN10bDac();
    _kasic["PP10bDac"] = getPP10bDac();
    _kasic["EN_adc"] = getEN_adc();
    _kasic["PP_adc"] = getPP_adc();
    _kasic["sel_starb_ramp_adc_ext"] = getsel_starb_ramp_adc_ext();
    _kasic["usebcompensation"] = getusebcompensation();
    _kasic["EN_bias_dac_delay"] = getEN_bias_dac_delay();
    _kasic["PP_bias_dac_delay"] = getPP_bias_dac_delay();
    _kasic["EN_bias_ramp_delay"] = getEN_bias_ramp_delay();
    _kasic["PP_bias_ramp_delay"] = getPP_bias_ramp_delay();
    _kasic["EN_discri_delay"] = getEN_discri_delay();
    _kasic["PP_discri_delay"] = getPP_discri_delay();
    _kasic["EN_temp_sensor"] = getEN_temp_sensor();
    _kasic["PP_temp_sensor"] = getPP_temp_sensor();
    _kasic["EN_bias_pa"] = getEN_bias_pa();
    _kasic["PP_bias_pa"] = getPP_bias_pa();
    _kasic["EN_bias_discri"] = getEN_bias_discri();
    _kasic["PP_bias_discri"] = getPP_bias_discri();
    _kasic["cmd_polarity"] = getcmd_polarity();
    _kasic["latch"] = getlatch();
    _kasic["EN_bias_6bit_dac"] = getEN_bias_6bit_dac();
    _kasic["PP_bias_6bit_dac"] = getPP_bias_6bit_dac();
    _kasic["EN_bias_tdc"] = getEN_bias_tdc();
    _kasic["PP_bias_tdc"] = getPP_bias_tdc();
    _kasic["ON_OFF_input_dac"] = getON_OFF_input_dac();
    _kasic["EN_bias_charge"] = getEN_bias_charge();
    _kasic["PP_bias_charge"] = getPP_bias_charge();
    _kasic["Cf3_100fF"] = getCf3_100fF();
    _kasic["Cf2_200fF"] = getCf2_200fF();
    _kasic["Cf1_2p5pF"] = getCf1_2p5pF();
    _kasic["Cf0_1p25pF"] = getCf0_1p25pF();
    _kasic["EN_bias_sca"] = getEN_bias_sca();
    _kasic["PP_bias_sca"] = getPP_bias_sca();
    _kasic["EN_bias_discri_charge"] = getEN_bias_discri_charge();
    _kasic["PP_bias_discri_charge"] = getPP_bias_discri_charge();
    _kasic["EN_bias_discri_adc_time"] = getEN_bias_discri_adc_time();
    _kasic["PP_bias_discri_adc_time"] = getPP_bias_discri_adc_time();
    _kasic["EN_bias_discri_adc_charge"] = getEN_bias_discri_adc_charge();
    _kasic["PP_bias_discri_adc_charge"] = getPP_bias_discri_adc_charge();
    _kasic["DIS_razchn_int"] = getDIS_razchn_int();
    _kasic["DIS_razchn_ext"] = getDIS_razchn_ext();
    _kasic["SEL_80M"] = getSEL_80M();
    _kasic["EN_80M"] = getEN_80M();
    _kasic["EN_slow_lvds_rec"] = getEN_slow_lvds_rec();
    _kasic["PP_slow_lvds_rec"] = getPP_slow_lvds_rec();
    _kasic["EN_fast_lvds_rec"] = getEN_fast_lvds_rec();
    _kasic["PP_fast_lvds_rec"] = getPP_fast_lvds_rec();
    _kasic["EN_transmitter"] = getEN_transmitter();
    _kasic["PP_transmitter"] = getPP_transmitter();
    _kasic["ON_OFF_1mA"] = getON_OFF_1mA();
    _kasic["ON_OFF_2mA"] = getON_OFF_2mA();
    _kasic["ON_OFF_otaQ"] = getON_OFF_otaQ();
    _kasic["ON_OFF_ota_mux"] = getON_OFF_ota_mux();
    _kasic["ON_OFF_ota_probe"] = getON_OFF_ota_probe();
    _kasic["DIS_trig_mux"] = getDIS_trig_mux();
    _kasic["EN_NOR32_time"] = getEN_NOR32_time();
    _kasic["EN_NOR32_charge"] = getEN_NOR32_charge();
    _kasic["DIS_triggers"] = getDIS_triggers();
    _kasic["EN_dout_oc"] = getEN_dout_oc();
    _kasic["EN_transmit"] = getEN_transmit();
    _kasic["PA_ccomp_0"] = getPA_ccomp_0();
    _kasic["PA_ccomp_1"] = getPA_ccomp_1();
    _kasic["PA_ccomp_2"] = getPA_ccomp_2();
    _kasic["PA_ccomp_3"] = getPA_ccomp_3();
    _kasic["Choice_Trigger_Out"] = getChoice_Trigger_Out();
    _kasic["DacDelay"] = getDacDelay();
    _kasic["Delay_reset_trigger"]=getDelay_reset_trigger();
    _kasic["En_reset_trigger_delay"]= ((int)getEn_reset_trigger_delay());
    _kasic["Delay_reset_ToT"]=getDelay_reset_ToT();
    _kasic["En_reset_ToT_delay"]= ((int)getEn_reset_ToT_delay());

    web::json::value idac;
    web::json::value bdac;
    web::json::value mdc;
    web::json::value mdt;
    web::json::value idc;
    for (int ch = 0; ch < 32; ch++)
    {
      idac[ch]=((int)getInputDac(ch));
      bdac[ch]=((int)get6bDac(ch));
      mdc[ch]=((int)getMaskDiscriCharge(ch));
      mdt[ch]=((int)getMaskDiscriTime(ch));
      idc[ch]=((int)getInputDacCommand(ch));
    }
    _kasic["InputDac"] = idac;
    _kasic["6bDac"] = bdac;
    _kasic["MaskDiscriCharge"] = mdc;
    _kasic["MaskDiscriTime"] = mdt;
    _kasic["InputDacCommand"] = idc;

    _kasic["VthDiscriCharge"] = getVthDiscriCharge();
    _kasic["VthTime"] = getVthTime();
    
    _jasic=_kasic;
  }

  ///  Dump JSON variable
  void dumpJson()
  {
    std::cout << _jasic << std::endl;
  }

  ///  Load JSON froma  file
  void loadJson(std::string fname)
  {
    
    web::json::value output;  // JSON read from input file

      try
	{
	  // Open the file stream
	  std::ifstream f(fname);
	  // String stream for holding the JSON file
	  std::stringstream strStream;

	  // Stream file stream into string stream
	  strStream << f.rdbuf();
	  f.close();  // Close the filestream
	  
	  // Parse the string stream into a JSON object
	  output = web::json::value::parse(strStream);
	}
      catch (web::json::json_exception excep)
	{
	  _jasic=web::json::value::null();
	  //throw web::json::json_exception("Error Parsing JSON file " + jsonFileName);
	}
      _jasic=output;

  }

  ///  set value from the JSON load
  void setFromJson()
  {


    uint8_t ch = 0;
    for (auto it = _jasic["6bDac"].as_array().begin(); it != _jasic["6bDac"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      set6bDac(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["InputDac"].as_array().begin(); it != _jasic["InputDac"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setInputDac(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["InputDacCommand"].as_array().begin(); it != _jasic["InputDacCommand"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setInputDacCommand(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["MaskDiscriCharge"].as_array().begin(); it != _jasic["MaskDiscriCharge"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMaskDiscriCharge(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["MaskDiscriTime"].as_array().begin(); it != _jasic["MaskDiscriTime"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMaskDiscriTime(ch, sid);
      ch++;
    }

    
    setCf0_1p25pF(_jasic["Cf0_1p25pF"].as_integer());
    setCf1_2p5pF(_jasic["Cf1_2p5pF"].as_integer());
    setCf2_200fF(_jasic["Cf2_200fF"].as_integer());
    setCf3_100fF(_jasic["Cf3_100fF"].as_integer());
    setDIS_razchn_ext(_jasic["DIS_razchn_ext"].as_integer());
    setDIS_razchn_int(_jasic["DIS_razchn_int"].as_integer());
    setDIS_trig_mux(_jasic["DIS_trig_mux"].as_integer());
    setDIS_triggers(_jasic["DIS_triggers"].as_integer());
    setDacDelay(_jasic["DacDelay"].as_integer());
    setEN10bDac(_jasic["EN10bDac"].as_integer());
    setEN_80M(_jasic["EN_80M"].as_integer());
    setEN_NOR32_charge(_jasic["EN_NOR32_charge"].as_integer());
    setEN_NOR32_time(_jasic["EN_NOR32_time"].as_integer());
    setEN_adc(_jasic["EN_adc"].as_integer());
    setEN_bias_6bit_dac(_jasic["EN_bias_6bit_dac"].as_integer());
    setEN_bias_charge(_jasic["EN_bias_charge"].as_integer());
    setEN_bias_dac_delay(_jasic["EN_bias_dac_delay"].as_integer());
    setEN_bias_discri(_jasic["EN_bias_discri"].as_integer());
    setEN_bias_discri_adc_charge(_jasic["EN_bias_discri_adc_charge"].as_integer());
    setEN_bias_discri_adc_time(_jasic["EN_bias_discri_adc_time"].as_integer());
    setEN_bias_discri_charge(_jasic["EN_bias_discri_charge"].as_integer());
    setEN_bias_pa(_jasic["EN_bias_pa"].as_integer());
    setEN_bias_ramp_delay(_jasic["EN_bias_ramp_delay"].as_integer());
    setEN_bias_sca(_jasic["EN_bias_sca"].as_integer());
    setEN_bias_tdc(_jasic["EN_bias_tdc"].as_integer());
    setEN_discri_delay(_jasic["EN_discri_delay"].as_integer());
    setEN_dout_oc(_jasic["EN_dout_oc"].as_integer());
    setEN_fast_lvds_rec(_jasic["EN_fast_lvds_rec"].as_integer());
    setEN_slow_lvds_rec(_jasic["EN_slow_lvds_rec"].as_integer());
    setEN_temp_sensor(_jasic["EN_temp_sensor"].as_integer());
    setEN_transmit(_jasic["EN_transmit"].as_integer());
    setEN_transmitter(_jasic["EN_transmitter"].as_integer());
    setON_OFF_1mA(_jasic["ON_OFF_1mA"].as_integer());
    setON_OFF_2mA(_jasic["ON_OFF_2mA"].as_integer());
    setON_OFF_input_dac(_jasic["ON_OFF_input_dac"].as_integer());
    setON_OFF_otaQ(_jasic["ON_OFF_otaQ"].as_integer());
    setON_OFF_ota_mux(_jasic["ON_OFF_ota_mux"].as_integer());
    setON_OFF_ota_probe(_jasic["ON_OFF_ota_probe"].as_integer());
    setPP10bDac(_jasic["PP10bDac"].as_integer());
    setPP_adc(_jasic["PP_adc"].as_integer());
    setPP_bias_6bit_dac(_jasic["PP_bias_6bit_dac"].as_integer());
    setPP_bias_charge(_jasic["PP_bias_charge"].as_integer());
    setPP_bias_dac_delay(_jasic["PP_bias_dac_delay"].as_integer());
    setPP_bias_discri(_jasic["PP_bias_discri"].as_integer());
    setPP_bias_discri_adc_charge(_jasic["PP_bias_discri_adc_charge"].as_integer());
    setPP_bias_discri_adc_time(_jasic["PP_bias_discri_adc_time"].as_integer());
    setPP_bias_discri_charge(_jasic["PP_bias_discri_charge"].as_integer());
    setPP_bias_pa(_jasic["PP_bias_pa"].as_integer());
    setPP_bias_ramp_delay(_jasic["PP_bias_ramp_delay"].as_integer());
    setPP_bias_sca(_jasic["PP_bias_sca"].as_integer());
    setPP_bias_tdc(_jasic["PP_bias_tdc"].as_integer());
    setPP_discri_delay(_jasic["PP_discri_delay"].as_integer());
    setPP_fast_lvds_rec(_jasic["PP_fast_lvds_rec"].as_integer());
    setPP_slow_lvds_rec(_jasic["PP_slow_lvds_rec"].as_integer());
    setPP_temp_sensor(_jasic["PP_temp_sensor"].as_integer());
    setPP_transmitter(_jasic["PP_transmitter"].as_integer());
    setSEL_80M(_jasic["SEL_80M"].as_integer());
    setVthDiscriCharge(_jasic["VthDiscriCharge"].as_integer());
    setVthTime(_jasic["VthTime"].as_integer());
    setcmd_polarity(_jasic["cmd_polarity"].as_integer());
    setlatch(_jasic["latch"].as_integer());
    setsel_starb_ramp_adc_ext(_jasic["sel_starb_ramp_adc_ext"].as_integer());
    setusebcompensation(_jasic["usebcompensation"].as_integer());
    // 2B
    setPA_ccomp_0(_jasic["PA_ccomp_0"].as_integer());
    setPA_ccomp_1(_jasic["PA_ccomp_1"].as_integer());
    setPA_ccomp_1(_jasic["PA_ccomp_2"].as_integer());
    setPA_ccomp_1(_jasic["PA_ccomp_3"].as_integer());
    setChoice_Trigger_Out(_jasic["Choice_Trigger_Out"].as_integer());
    // 2C
    setDelay_reset_trigger(_jasic["Delay_reset_trigger"].as_integer());
    setEn_reset_trigger_delay(_jasic["En_reset_trigger_delay"].as_integer());
    setDelay_reset_ToT(_jasic["Delay_reset_ToT"].as_integer());
    setEn_reset_ToT_delay(_jasic["En_reset_ToT_delay"].as_integer());

  
    
  }
  ///  Pointer to bit array
  uint32_t *ptr() { return _l; }
  /**
     * \brief Setters and getters (see PETIROC Manuals for details)
     * */
  /// Now real access to register function
  bool getMaskDiscriCharge(int b) { return getBit(b % 32); }
  void setMaskDiscriCharge(int b, bool on) { setBitState(b, on); }
  uint8_t getInputDac(uint8_t ch) { return getByte(32 + ch * 9); }
  void setInputDac(uint8_t ch, uint8_t val) { setByte(32 + ch * 9, val, 8); }
  bool getInputDacCommand(int ch) { return getBit(32 + ch * 9 + 8); }
  void setInputDacCommand(int ch, bool on) { setBitState(32 + ch * 9 + 8, on); }

  bool getMaskDiscriTime(int b) { return getBit(328 + b); }
  void setMaskDiscriTime(int b, bool on) { setBitState(328 + b, on); }
  uint8_t get6bDac(uint8_t ch) { return getByte(360 + ch * 6) & 0x3F; }
  void set6bDac(uint8_t ch, uint8_t val) { setByte(360 + ch * 6, val & 0x3F, 6); }
  bool getEN10bDac() { return getBit(552); }
  void setEN10bDac(bool on) { setBitState(552, on); }
  bool getPP10bDac() { return getBit(553); }
  void setPP10bDac(bool on) { setBitState(553, on); }

  uint16_t getVthDiscriCharge()
  {
    uint16_t r = 0;
    for (int i = 1; i <= 10; i++)
      if (getBit(554 + i - 1))
        r |= (1 << (10 - i));
    return r;
  }
  void setVthDiscriCharge(uint16_t val)
  {
    uint16_t r = val & 0x3FF;
    for (int i = 1; i <= 10; i++)
      setBitState(554 + i - 1, r & (1 << (10 - i)));
  }
  uint16_t getVthTime()
  {
    uint16_t r = 0;
    for (int i = 1; i <= 10; i++)
      if (getBit(564 + i - 1))
        r |= (1 << (10 - i));
    return r;
  }
  void setVthTime(uint16_t val)
  {
    uint16_t r = val & 0x3FF;
    for (int i = 1; i <= 10; i++)
      setBitState(564 + i - 1, r & (1 << (10 - i)));
  }

  
  uint8_t getDacDelay() { return getByte(582); }
  void setDacDelay(uint8_t val) { setByte(582, val, 8); }
  void setEN_adc(bool t) { setBitState(574, t); }
  void setPP_adc(bool t) { setBitState(575, t); }
  void setsel_starb_ramp_adc_ext(bool t) { setBitState(576, t); }
  void setusebcompensation(bool t) { setBitState(577, t); }
  void setEN_bias_dac_delay(bool t) { setBitState(578, t); }
  void setPP_bias_dac_delay(bool t) { setBitState(579, t); }
  void setEN_bias_ramp_delay(bool t) { setBitState(580, t); }
  void setPP_bias_ramp_delay(bool t) { setBitState(581, t); }
  void setEN_discri_delay(bool t) { setBitState(590, t); }
  void setPP_discri_delay(bool t) { setBitState(591, t); }
  void setEN_temp_sensor(bool t) { setBitState(592, t); }
  void setPP_temp_sensor(bool t) { setBitState(593, t); }
  void setEN_bias_pa(bool t) { setBitState(594, t); }
  void setPP_bias_pa(bool t) { setBitState(595, t); }
  void setEN_bias_discri(bool t) { setBitState(596, t); }
  void setPP_bias_discri(bool t) { setBitState(597, t); }
  void setcmd_polarity(bool t) { setBitState(598, t); }
  void setlatch(bool t) { setBitState(599, t); }
  void setEN_bias_6bit_dac(bool t) { setBitState(600, t); }
  void setPP_bias_6bit_dac(bool t) { setBitState(601, t); }
  void setEN_bias_tdc(bool t) { setBitState(602, t); }
  void setPP_bias_tdc(bool t) { setBitState(603, t); }
  void setON_OFF_input_dac(bool t) { setBitState(604, t); }
  void setEN_bias_charge(bool t) { setBitState(605, t); }
  void setPP_bias_charge(bool t) { setBitState(606, t); }
  void setCf3_100fF(bool t) { setBitState(607, t); }
  void setCf2_200fF(bool t) { setBitState(608, t); }
  void setCf1_2p5pF(bool t) { setBitState(609, t); }
  void setCf0_1p25pF(bool t) { setBitState(610, t); }
  void setEN_bias_sca(bool t) { setBitState(611, t); }
  void setPP_bias_sca(bool t) { setBitState(612, t); }
  void setEN_bias_discri_charge(bool t) { setBitState(613, t); }
  void setPP_bias_discri_charge(bool t) { setBitState(614, t); }
  void setEN_bias_discri_adc_time(bool t) { setBitState(615, t); }
  void setPP_bias_discri_adc_time(bool t) { setBitState(616, t); }
  void setEN_bias_discri_adc_charge(bool t) { setBitState(617, t); }
  void setPP_bias_discri_adc_charge(bool t) { setBitState(618, t); }
  void setDIS_razchn_int(bool t) { setBitState(619, t); }
  void setDIS_razchn_ext(bool t) { setBitState(620, t); }
  void setSEL_80M(bool t) { setBitState(621, t); }
  void setEN_80M(bool t) { setBitState(622, t); }
  void setEN_slow_lvds_rec(bool t) { setBitState(623, t); }
  void setPP_slow_lvds_rec(bool t) { setBitState(624, t); }
  void setEN_fast_lvds_rec(bool t) { setBitState(625, t); }
  void setPP_fast_lvds_rec(bool t) { setBitState(626, t); }
  void setEN_transmitter(bool t) { setBitState(627, t); }
  void setPP_transmitter(bool t) { setBitState(628, t); }
  void setON_OFF_1mA(bool t) { setBitState(629, t); }
  void setON_OFF_2mA(bool t) { setBitState(630, t); }
  void setON_OFF_otaQ(bool t) { setBitState(631, t); }
  void setON_OFF_ota_mux(bool t) { setBitState(632, t); }
  void setON_OFF_ota_probe(bool t) { setBitState(633, t); }
  void setDIS_trig_mux(bool t) { setBitState(634, t); }
  void setEN_NOR32_time(bool t) { setBitState(635, t); }
  void setEN_NOR32_charge(bool t) { setBitState(636, t); }
  void setDIS_triggers(bool t) { setBitState(637, t); }
  void setEN_dout_oc(bool t) { setBitState(638, t); }
  void setEN_transmit(bool t) { setBitState(639, t); }
  void setPA_ccomp_0(bool t) { setBitState(640, t); }
  void setPA_ccomp_1(bool t) { setBitState(641, t); }
  void setPA_ccomp_2(bool t) { setBitState(642, t); }
  void setPA_ccomp_3(bool t) { setBitState(643, t); }
  void setChoice_Trigger_Out(bool t) { setBitState(647, t); }

  
  bool getEN_adc() { return getBit(574); }
  bool getPP_adc() { return getBit(575); }
  bool getsel_starb_ramp_adc_ext() { return getBit(576); }
  bool getusebcompensation() { return getBit(577); }
  bool getEN_bias_dac_delay() { return getBit(578); }
  bool getPP_bias_dac_delay() { return getBit(579); }
  bool getEN_bias_ramp_delay() { return getBit(578); }
  bool getPP_bias_ramp_delay() { return getBit(579); }
  bool getEN_discri_delay() { return getBit(590); }
  bool getPP_discri_delay() { return getBit(591); }
  bool getEN_temp_sensor() { return getBit(592); }
  bool getPP_temp_sensor() { return getBit(593); }
  bool getEN_bias_pa() { return getBit(594); }
  bool getPP_bias_pa() { return getBit(595); }
  bool getEN_bias_discri() { return getBit(596); }
  bool getPP_bias_discri() { return getBit(597); }
  bool getcmd_polarity() { return getBit(598); }
  bool getlatch() { return getBit(599); }
  bool getEN_bias_6bit_dac() { return getBit(600); }
  bool getPP_bias_6bit_dac() { return getBit(601); }
  bool getEN_bias_tdc() { return getBit(602); }
  bool getPP_bias_tdc() { return getBit(603); }
  bool getON_OFF_input_dac() { return getBit(604); }
  bool getEN_bias_charge() { return getBit(605); }
  bool getPP_bias_charge() { return getBit(606); }
  bool getCf3_100fF() { return getBit(607); }
  bool getCf2_200fF() { return getBit(608); }
  bool getCf1_2p5pF() { return getBit(609); }
  bool getCf0_1p25pF() { return getBit(610); }
  bool getEN_bias_sca() { return getBit(611); }
  bool getPP_bias_sca() { return getBit(612); }
  bool getEN_bias_discri_charge() { return getBit(613); }
  bool getPP_bias_discri_charge() { return getBit(614); }
  bool getEN_bias_discri_adc_time() { return getBit(615); }
  bool getPP_bias_discri_adc_time() { return getBit(616); }
  bool getEN_bias_discri_adc_charge() { return getBit(617); }
  bool getPP_bias_discri_adc_charge() { return getBit(618); }
  bool getDIS_razchn_int() { return getBit(619); }
  bool getDIS_razchn_ext() { return getBit(620); }
  bool getSEL_80M() { return getBit(621); }
  bool getEN_80M() { return getBit(622); }
  bool getEN_slow_lvds_rec() { return getBit(623); }
  bool getPP_slow_lvds_rec() { return getBit(624); }
  bool getEN_fast_lvds_rec() { return getBit(625); }
  bool getPP_fast_lvds_rec() { return getBit(626); }
  bool getEN_transmitter() { return getBit(627); }
  bool getPP_transmitter() { return getBit(628); }
  bool getON_OFF_1mA() { return getBit(629); }
  bool getON_OFF_2mA() { return getBit(630); }
  bool getON_OFF_otaQ() { return getBit(631); }
  bool getON_OFF_ota_mux() { return getBit(632); }
  bool getON_OFF_ota_probe() { return getBit(633); }
  bool getDIS_trig_mux() { return getBit(634); }
  bool getEN_NOR32_time() { return getBit(635); }
  bool getEN_NOR32_charge() { return getBit(636); }
  bool getDIS_triggers() { return getBit(637); }
  bool getEN_dout_oc() { return getBit(638); }
  bool getEN_transmit() { return getBit(639); }
  bool getPA_ccomp_0() { return getBit(640); }
  bool getPA_ccomp_1() { return getBit(641); }
  bool getPA_ccomp_2() { return getBit(642); }
  bool getPA_ccomp_3() { return getBit(643); }
  bool getChoice_Trigger_Out() { return getBit(647); }
  
  uint8_t getDelay_reset_trigger() { return (getByte(648)&0xF);}
  void setDelay_reset_trigger(uint8_t v)  {setByte(648,(v&0xF),4);}
  bool getEn_reset_trigger_delay() { return getBit(655); }
  void setEn_reset_trigger_delay(bool t) { setBitState(655, t); }

  
  uint8_t getDelay_reset_ToT() { return (getByte(656)&0xF);}
  void setDelay_reset_ToT(uint8_t v)  {setByte(656,(v&0xF),4);}
  bool getEn_reset_ToT_delay() { return getBit(663); }
  void setEn_reset_ToT_delay(bool t) { setBitState(663, t); }

  /// Ivert bit orders
  void loadInvert(PRCSlow r)
  {

    for (int i = 647; i >= 0; i--)
      setBitState(647 - i, r.getBit(i) == 1);
  }

  

  /// Return JSON value
  web::json::value &getJson() { return _jasic; }

  /// Set JSON value
  void setJson(web::json::value v)
  {
    _jasic = v;
    setFromJson();
  }

private:
  uint32_t _l[21]; ///< 672 bits of SLC
  web::json::value _jasic; ///< JSON cpp Value 
};
