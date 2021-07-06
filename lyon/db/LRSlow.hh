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
#define SLC_BYTES_LENGTH 81
/**
 * 
 * \file LRSlow.hh
 * \brief  Access to LIROC slow control information
 * \author L.Mirabito
 * \version 1.0
 * */
using namespace std;
/**
 * \brief Class to manipulate slow control parameters of LIROC
 * */
#define LRNCH 64
#define LRCHSIZE 2
#define CHSHIFT 128
class LRSlow
{
public:
  ///  Constructor
  LRSlow() { memset(_l, 0, 139 * sizeof(uint8_t)); }
  ///  Get one bit status
  bool getBit(uint8_t w, int b) { return (_l[w] >> b) & 1; }
  /// set one bit
  void setBit(uint8_t w, int b) { _l[w] |= (1 << b); };
  ///  clear one bit
  void clearBit(uint8_t w, int b) { _l[w] &= ~(1 << b); };
  ///  Set the bit state
  void setBitState(uint8_t w, int b, bool t)
  {
    if (t)
      setBit(w,b);
    else
      clearBit(w,b);
  }

  void setWord(uint8_t w, int b, uint8_t val, uint8_t len, bool reverse = false)
  {

    if (reverse)
    {
      int shift = 0;
      for (int i = b + len - 1; i >= b; i--)
      {
        if ((val >> shift) & 1)
          setBit(w,i);
        else
          clearBit(w,i);
	shift++;
      }
    }
    else
    {
      int shift = 0;
      for (int i = b; i < b + len; i++)
      {
        if ((val >> shift) & 1)
          setBit(w,i);
        else
          clearBit(w,i);
	shift++;
      }
    }

    return;
  }

uint8_t getWord(uint8_t w,int b,int len,bool reverse=false)
  {
    uint8_t v=0;
    if (reverse)
      {
        int shift=0;
        for (int i=b+len-1;i>=b;i--)
          {v+=getBit(w,i)<<shift;shift++;}
      }
    else
      {
        int shift=0;
        for (int i=b;i<b+len;i++)
          {v+=getBit(w,i)<<shift;shift++;}
      }
    return v;
  }

  ///  Set at most 8 bits of a byte at b postion
  // Channel dependant values
  uint8_t getDC_pa(uint8_t ch) { return (_l[ch*2+0]>>2) & 0x3F; }
  bool getCtest(uint8_t ch) { return getBit(ch*2+0,1); }
  bool getMask(uint8_t ch) { return getBit(ch*2+1,7); }
  uint8_t getDAC_local(uint8_t ch) { return (uint8_t)(_l[ch*2+1] & 0x7F); }

  void setDC_pa(uint8_t ch, uint8_t val) { setWord(ch*2+0, 2,val & 0x3F, 6); }
  void setCtest(uint8_t ch, bool t) { setBitState(ch*2+0, 1, t); }
  void setMask(uint8_t ch, bool t) { setBitState(ch*2+1,7, t); }
  void setDAC_local(uint8_t ch, uint8_t val) { setWord( ch*2+1,0,val & 0x7F, 7); }  
  // Global
  //64|0
  bool getEN_pa() { return getBit(128+0,7); }
  bool getPP_pa() { return getBit(128+0,6); }
  uint8_t getPA_gain() { return (uint8_t) (_l[128+0]>>2) & 0xF; }
  void setEN_pa(bool t) { setBitState(128+0,7, t); }
  void setPP_pa(bool t) { setBitState(128+0,6, t); }
  void setPA_gain(uint8_t v) { setWord(128+0,2, v & 0xF, 4); }

  //64 |1
  bool getEN_7b() { return getBit(128+1,7); }
  bool getPP_7b() { return getBit(128+1,6); }
  void setEN_7b(bool t) { setBitState(128+1,7, t); }
  void setPP_7b(bool t) { setBitState(128+1,6, t); }

  //64 |2
  bool getEN_disc() { return getBit(128+2,7); }
  bool getPP_disc() { return getBit(128+2,6); }
  bool getPolarity() { return getBit(128+2,5); }
  bool getHysteresys() { return getBit(128+2,4); }
  void setEN_disc(bool t) { setBitState(128+2,7, t); }
  void setPP_disc(bool t) { setBitState(128+2,6, t); }
  void setPolarity(bool t) { setBitState(128+2,5, t); }
  void setHysteresys(bool t) { setBitState(128+2,4, t); }

  // 65| 0
  bool getEN_bg() { return getBit(131+0,7); }
  bool getPP_bg() { return getBit(131+0,6); }
  void setEN_bg(bool t) { setBitState(131+0,7, t); }
  void setPP_bg(bool t) { setBitState(131+0,6, t); }

  // 65 | 1
  bool getEN_10bDAC() { return getBit(131+1,7); }
  bool getPP_10bDAC() { return getBit(131+1,6); }
  void setEN_10bDAC(bool t) { setBitState(131+1,7, t); }
  void setPP_10bDAC(bool t) { setBitState(131+1,6, t); }

  // 65 | 2 DAC threhsold
  uint16_t getDAC_threshold() { uint16_t r=((_l[131+1]&0x3)<<8)|_l[131+2]; return r;}
  void setDAC_threshold(uint16_t v) { setWord(131+1, 0,(v>>8) & 0x3, 2); _l[131+2]=(v&0xFF); }

  // 66| 0
  uint8_t getCLPS_bsize() { return (uint8_t)getWord(134+0, 4,4, true) & 0xF; }
  uint8_t getEN_pre_emphasis() { return (uint8_t)getWord(134+0, 0,4, true) & 0xF; }

  void setCLPS_bsize(uint8_t v) { setWord(134+0,4, v & 0xF, 4, true); }
  void setEN_pre_emphasis(uint8_t v) { setWord(134+0,0, v & 0xF, 4, true); }

  // 66 |1
  uint8_t getPre_emphasis_delay() { return (uint8_t)getWord(134+1,6, 2, true); }
  void setPre_emphasis_delay(uint8_t v) { setWord(134+1,6, v & 0x4, 2, true); }

  // 66 | 2
  bool getEN_differential() { return getBit(134+2,7); }
  bool getPP_differential() { return getBit(134+2,6); }
  bool getForced_ValEvt() { return getBit(134+2,5); }
  void setEN_differential(bool t) { setBitState(134+2,7, t); }
  void setPP_differential(bool t) { setBitState(134+2,6, t); }
  void setForced_ValEvt(bool t) { setBitState(134+2,5, t); }

  // 67 | 0
  bool getEN_probe() { return getBit(137+0,7); }
  bool getPP_probe() { return getBit(137+0,6); }
  uint8_t getMillerComp() { return (uint8_t)_l[137+0]& 0x7; }
  void setEN_probe(bool t) { setBitState(137+0,7, t); }
  void setPP_probe(bool t) { setBitState(137+0,6, t); }
  void setMillerComp(uint8_t v) { setWord(137+0,0, v & 0x7, 3); }

  // 67 | 1
  uint8_t getIbi_probe() { return (uint8_t)getWord(137+1,6, 2) & 0x3; }
  uint8_t getIbo_probe() { return (uint8_t)getWord(137+1,0, 6) & 0x7F; }
  void setIbi_probe(uint8_t v) { setWord(137+1,6, v & 0x3, 2); }
  void setIbo_probe(uint8_t v) { setWord(137+1,0, v & 0x7F, 6); }


  ///  Print Slow Control
  void Print()
  {

    for (int ch = 0; ch < 64; ch++)
      std::cout << "DC_PA[" << ch << "]=" << (int)getDC_pa(ch) << std::endl;
    for (int ch = 0; ch < 64; ch++)
      std::cout << "Ctest[" << ch << "]=" << (int)getCtest(ch) << std::endl;

    for (int ch = 0; ch < 64; ch++)
      std::cout << "Mask[" << ch << "]=" << (int)getMask(ch) << std::endl;
    for (int ch = 0; ch < 64; ch++)
      std::cout << "DAC_local[" << ch << "]=" << (int)getDAC_local(ch) << std::endl;

    std::cout << "EN_pa =" << (int) getEN_pa() << std::endl;
    std::cout << "PP_pa =" << (int) getPP_pa() << std::endl;
    std::cout << "PA_gain =" << (int) getPA_gain() << std::endl;
    std::cout << "EN_7b =" << (int) getEN_7b() << std::endl;
    std::cout << "PP_7b =" << (int) getPP_7b() << std::endl;
    std::cout << "EN_disc =" << (int) getEN_disc() << std::endl;
    std::cout << "PP_disc =" << (int) getPP_disc() << std::endl;
    std::cout << "Polarity =" << (int) getPolarity() << std::endl;
    std::cout << "Hysteresys =" << (int) getHysteresys() << std::endl;
    std::cout << "EN_bg =" << (int) getEN_bg() << std::endl;
    std::cout << "PP_bg =" << (int) getPP_bg() << std::endl;
    std::cout << "EN_10bDAC =" << (int) getEN_10bDAC() << std::endl;
    std::cout << "PP_10bDAC =" << (int) getPP_10bDAC() << std::endl;
    std::cout << "DAC_threshold =" << (int) getDAC_threshold() << std::endl;

    std::cout << "CLPS_bsize =" << (int) getCLPS_bsize() << std::endl;
    std::cout << "EN_pre_emphasis =" << (int) getEN_pre_emphasis() << std::endl;
    std::cout << "Pre_emphasis_delay =" << (int) getPre_emphasis_delay() << std::endl;
    std::cout << "EN_differential =" << (int) getEN_differential() << std::endl;
    std::cout << "PP_differential =" << (int) getPP_differential() << std::endl;
    std::cout << "Forced_ValEvt =" << (int) getForced_ValEvt() << std::endl;
    std::cout << "EN_probe =" << (int) getEN_probe() << std::endl;
    std::cout << "PP_probe =" << (int) getPP_probe() << std::endl;
    std::cout << "MillerComp =" << (int) getMillerComp() << std::endl;
    std::cout << "Ibi_probe =" << (int) getIbi_probe() << std::endl;
    std::cout << "Ibo_probe =" << (int) getIbo_probe() << std::endl;

    for (int i = 0; i < 139; i++)
    {
      if (i % 8 == 0)
        printf("\n");
      printf("%.4x ", _l[i]);
    }
  }

  ///  store in JSON SLC values
  void toJson()
  {
    //_jasic.clear();
    web::json::value _kasic;
    _kasic["EN_pa"] = getEN_pa();
    _kasic["PP_pa"] = getPP_pa();
    _kasic["PA_gain"] = getPA_gain();
    _kasic["EN_7b"] = getEN_7b();
    _kasic["PP_7b"] = getPP_7b();
    _kasic["EN_disc"] = getEN_disc();
    _kasic["PP_disc"] = getPP_disc();
    _kasic["Polarity"] = getPolarity();
    _kasic["Hysteresys"] = getHysteresys();
    _kasic["EN_bg"] = getEN_bg();
    _kasic["PP_bg"] = getPP_bg();
    _kasic["EN_10bDAC"] = getEN_10bDAC();
    _kasic["PP_10bDAC"] = getPP_10bDAC();
    _kasic["DAC_threshold"] = getDAC_threshold();

    _kasic["CLPS_bsize"] = getCLPS_bsize();
    _kasic["EN_pre_emphasis"] = getEN_pre_emphasis();
    _kasic["Pre_emphasis_delay"] = getPre_emphasis_delay();
    _kasic["EN_differential"] = getEN_differential();
    _kasic["PP_differential"] = getPP_differential();
    _kasic["Forced_ValEvt"] = getForced_ValEvt();
    _kasic["EN_probe"] = getEN_probe();
    _kasic["PP_probe"] = getPP_probe();
    _kasic["MillerComp"] = getMillerComp();
    _kasic["Ibi_probe"] = getIbi_probe();
    _kasic["Ibo_probe"] = getIbo_probe();

    web::json::value dc_pa;
    web::json::value ctest;
    web::json::value mask;
    web::json::value dac_local;
    for (int ch = 0; ch < 64; ch++)
    {
      dc_pa[ch] = ((int)getDC_pa(ch));
      ctest[ch] = ((int)getCtest(ch));
      mask[ch] = ((int)getMask(ch));
      dac_local[ch] = ((int)getDAC_local(ch));
    }
    _kasic["DC_pa"] = dc_pa;
    _kasic["Ctest"] = ctest;
    _kasic["Mask"] = mask;
    _kasic["DAC_local"] = dac_local;
    _jasic = _kasic;
  }

  ///  Dump JSON variable
  void dumpJson()
  {
    std::cout << _jasic << std::endl;
  }

  ///  Load JSON froma  file
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
      _jasic = web::json::value::null();
      //throw web::json::json_exception("Error Parsing JSON file " + jsonFileName);
    }
    _jasic = output;
  }

  ///  set value from the JSON load
  void setFromJson()
  {

    uint8_t ch = 0;
    for (auto it = _jasic["DC_pa"].as_array().begin(); it != _jasic["DC_pa"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setDC_pa(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["Ctest"].as_array().begin(); it != _jasic["Ctest"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setCtest(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["Mask"].as_array().begin(); it != _jasic["Mask"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setMask(ch, sid);
      ch++;
    }

    ch = 0;
    for (auto it = _jasic["DAC_local"].as_array().begin(); it != _jasic["DAC_local"].as_array().end(); it++)
    {
      uint8_t sid = (*it).as_integer();
      setDAC_local(ch, sid);
      fprintf(stderr," ch %d val %d l %d \n",ch,sid,_l[2*ch+1]);
      ch++;
    }

    setEN_pa(_jasic["EN_pa"].as_integer());
    setPP_pa(_jasic["PP_pa"].as_integer());
    setPA_gain(_jasic["PA_gain"].as_integer());
    setEN_7b(_jasic["EN_7b"].as_integer());
    setPP_7b(_jasic["PP_7b"].as_integer());
    setEN_disc(_jasic["EN_disc"].as_integer());
    setPP_disc(_jasic["PP_disc"].as_integer());
    setPolarity(_jasic["Polarity"].as_integer());
    setHysteresys(_jasic["Hysteresys"].as_integer());
    setEN_bg(_jasic["EN_bg"].as_integer());
    setPP_bg(_jasic["PP_bg"].as_integer());
    setEN_10bDAC(_jasic["EN_10bDAC"].as_integer());
    setPP_10bDAC(_jasic["PP_10bDAC"].as_integer());
    setDAC_threshold(_jasic["DAC_threshold"].as_integer());

    setCLPS_bsize(_jasic["CLPS_bsize"].as_integer());
    setEN_pre_emphasis(_jasic["EN_pre_emphasis"].as_integer());
    setPre_emphasis_delay(_jasic["Pre_emphasis_delay"].as_integer());
    setEN_differential(_jasic["EN_differential"].as_integer());
    setPP_differential(_jasic["PP_differential"].as_integer());
    setForced_ValEvt(_jasic["Forced_ValEvt"].as_integer());
    setEN_probe(_jasic["EN_probe"].as_integer());
    setPP_probe(_jasic["PP_probe"].as_integer());
    setMillerComp(_jasic["MillerComp"].as_integer());
    setIbi_probe(_jasic["Ibi_probe"].as_integer());
    setIbo_probe(_jasic["Ibo_probe"].as_integer());
  }
  ///  Pointer to bit array
  uint8_t *ptr() { return _l; }

  uint32_t *board_ptr()
  {
    for (int i = 0; i < 139; i++)
    {
      uint8_t address, sub;
      if (i < 128)
      {
        address = i / 2;
        sub = i % 2;
      }
      else
      {
        int j = i - 128;
        address = 64 + j / 3;
        sub = j % 3;
      }
      _board[i] = _l[i] | (address << 8) | (sub << 16);
    }
    return _board;
  }
 


  /// Return JSON value
  web::json::value &getJson() { return _jasic; }

  /// Set JSON value
  void setJson(web::json::value v)
  {
    _jasic = v;
    std::cout<<_jasic<<std::endl;
    setFromJson();
  }

private:
  uint8_t _l[139]; ///< 640 bits of SLC
  uint32_t _board[139];
  web::json::value _jasic; ///< JSON cpp Value
};
