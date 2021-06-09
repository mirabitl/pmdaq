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
#define LRCHSIZE 16
#define CHSHIFT 1024
class LRSlow
{
public:
  ///  Constructor
  PRBSlow() { memset(_l, 0, 21 * sizeof(uint32_t)); }
  ///  Get one bit status
  bool getBit(int b) { return (_l[b / 8] >> (b % 8)) & 1; }
  /// set one bit
  void setBit(int b) { _l[b / 8] |= (1 << (b % 8)); };
  ///  clear one bit
  void clearBit(int b) { _l[b / 8] &= ~(1 << (b % 8)); };
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
  uint16_t getWord(int b,int len,bool reverse=false)
  {
    uint16_t v=0;
    if (!reverse)
      {
	int shift=0;
	for (int i=b+len-1;i>=b;i--)
	  {v+=getBit(i)<<shift;shift++;}
      }
    else
      {
	int shift=0;
	for (int i=b;i<b+len;i++)
	  {v+=getBit(i)<<shift;shift++;}
      }
    return v;
  }


  void setWord(int b, uint16_t val, uint8_t len,bool reverse=false)
  {

    
    if (!reverse)
      {
	int shift=0;
	for (int i=b+len-1;i>=b;i--)
	  {
	    if ((val >> shift) & 1)
	      setBit(i);
	    else
	      clearBit(i);
	  }
      }
    else
      {
	int shift=0;
	for (int i=b;i<b+len;i++)
	  {
	    if ((val >> shift) & 1)
	      setBit(i);
	    else
	      clearBit(i);
	  }
      }
	
    return;
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
  uint8_t getDC_PA(uint8_t ch){return (uint8_t) getWord(ch*16+0,6)&0x3F;}
  bool getCtest(uint8_t ch){return getBit(ch*16+6);}
  bool getMask(uint8_t ch){return getBit(ch*16+8);}
  uint8_t getDAC_local(uint8_t ch){return (uint8_t) getWord(ch*16+9,7)&0x7F;}
  bool getEN_pa(){return getBit(1024+0);}
  bool getPP_pa(){return getBit(1024+1);}
  uint8_t getPA_gain(){return (uint8_t) getWord(1024+2,4)&0xF;}
  bool getEN_7b(){return getBit(1032+0);}
  bool getPP_7b(){return getBit(1032+1);}
  bool getEN_disc(){return getBit(1040+0);}
  bool getPP_disc(){return getBit(1040+1);}
  bool getPolarity(){return getBit(1040+2);}
  bool getHysteresys(){return getBit(1040+3);}
  bool getEN_bg(){return getBit(1048+0);}
  bool getPP_bg(){return getBit(1048+1);}
  bool getEN_10bDAC(){return getBit(1056+0);}
  bool getPP_10bDAC(){return getBit(1056+1);}
  uint16_t getDAC_threshold(){return getWord(1056+6,10)&0x3FF;}

  uint8_t getCLPS_bsize(){return (uint8_t) getWord(1072,4,true)&0xF;}
  uint8_t getEN_pre_emphasis(){return (uint8_t) getWord(1072+4,4,true)&0xF;}
  uint8_t getPre_emphasis_delay(){return (uint8_t) getWord(1080,2,true);}

  bool getEN_differential(){return getBit(1088+0);}
  bool getPP_differential(){return getBit(1088+1);}
  bool getForced_ValEvt(){return getBit(1088+2);}
  bool getEN_probe(){return getBit(1096+0);}
  bool getPP_probe(){return getBit(1096+1);}
  uint8_t getMillerComp(){return (uint8_t) getWord(1096+5,3)&0xF;}
  uint8_t getIbi_probe(){return  (uint8_t) getWord(1104+0,2)&0xF;}
  uint8_t getIbo_probe(){return  (uint8_t) getWord(1104+2,6)&0xF;}


  void setDC_PA(uint8_t ch,uint8_t val){setWord(ch*16+0,val&0x3F,6);}
  void setCtest(uint8_t ch,bool t){setBitState(ch*16+6,t);}
  void setMask(uint8_t ch,bool t){setBitState(ch*16+8,t);}
  void setDAC_local(uint8_t ch,uint8_t val){setWord(ch*16+9,,val&0x7F,7);}
  void setEN_pa(bool t){setBitState(1024+0,t);}
  void setPP_pa(bool t){setBitState(1024+1,t);}
  void setPA_gain(  uint8_t v){setWord(1024+2,v&0xF,4);}
  void setEN_7b(bool t){setBitState(1032+0,t);}
  void setPP_7b(bool t){setBitState(1032+1,t);}
  void setEN_disc(bool t){setBitState(1040+0,t);}
  void setPP_disc(bool t){setBitState(1040+1,t);}
  void setPolarity(bool t){setBitState(1040+2,t);}
  void setHysteresys(bool t){setBitState(1040+3,t);}
  void setEN_bg(bool t){setBitState(1048+0,t);}
  void setPP_bg(bool t){setBitState(1048+1,t);}
  void setEN_10bDAC(bool t){setBitState(1056+0,t);}
  void setPP_10bDAC(bool t){setBitState(1056+1,t);}
  void setDAC_threshold(  uint16_t v){setWord(1056+6,v&0x3FF,10);}
  
  void setCLPS_bsize(  uint8_t v){setWord(1072,v&0xF,4,true);}
  void setEN_pre_emphasis(  uint8_t v){setWord(1072+4,v&0xF,4,true);}
  void setPre_emphasis_delay(  uint8_t v){setWord(1080,v&0x4,2,true);}
  
  void setEN_differential(bool t){setBitState(1088+0,t);}
  void setPP_differential(bool t){setBitState(1088+1,t);}
  void setForced_ValEvt(bool t){setBitState(1088+2,t);}
  void setEN_probe(bool t){setBitState(1096+0,t);}
  void setPP_probe(bool t){setBitState(1096+1,t);}
  void setMillerComp(  uint8_t v){setWord(1096+5,v&0x7,3);}
  void setIbi_probe(  uint8_t v){setWord(1104+0,v&0x3,2);}
  void setIbo_probe(  uint8_t v){setWord(1104+2,v&0x3f,6);}


  ///  Print Slow Control
  void Print()
  {

    for (int ch = 0; ch < 64; ch++)
      std::cout << "DC_PA[" << ch << "]=" << (int)getDC_PA(ch) << std::endl;
    for (int ch = 0; ch < 64; ch++)
      std::cout << "Ctest[" << ch << "]=" << (int)getCtest(ch) << std::endl;
    
    for (int ch = 0; ch < 64; ch++)
      std::cout << "Mask[" << ch << "]=" << (int)getMask(ch) << std::endl;
    for (int ch = 0; ch < 64; ch++)
      std::cout << "DAC_local[" << ch << "]=" << (int)getDAC_local(ch) << std::endl;

    std::cout << "EN_pa =" << getEN_pa() << std::endl;
    std::cout << "PP_pa =" << getPP_pa() << std::endl;
    std::cout << "PA_gain =" << getPA_gain() << std::endl;
    std::cout << "EN_7b =" << getEN_7b() << std::endl;
    std::cout << "PP_7b =" << getPP_7b() << std::endl;
    std::cout << "EN_disc =" << getEN_disc() << std::endl;
    std::cout << "PP_disc =" << getPP_disc() << std::endl;
    std::cout << "Polarity =" << getPolarity() << std::endl;
    std::cout << "Hysteresys =" << getHysteresis() << std::endl;
    std::cout << "EN_bg =" << getEN_bg() << std::endl;
    std::cout << "PP_bg =" << getPP_bg() << std::endl;
    std::cout << "EN_10bDAC =" << getEN_10bDAC() << std::endl;
    std::cout << "PP_10bDAC =" << getPP_10bDAC() << std::endl;
    std::cout << "DAC_threshold =" << getDAC_threshold() << std::endl;

    std::cout << "CLPS_bsize =" << getCLPS_bsize() << std::endl;
    std::cout << "EN_pre_emphasis =" << getEN_pre_emphasis() << std::endl;
    std::cout << "Pre_emphasis_delay =" << getPre_emphasis_delay() << std::endl;
    std::cout << "EN_differential =" << getEN_differential() << std::endl;
    std::cout << "PP_differential =" << getPP_differential() << std::endl;
    std::cout << "Forced_ValEvt =" << getForced_ValEvt() << std::endl;
    std::cout << "EN_probe =" << getEN_probe() << std::endl;
    std::cout << "PP_probe =" << getPP_probe() << std::endl;
    std::cout << "MillerComp =" << getMillerComp() << std::endl;
    std::cout << "Ibi_probe =" << getIbi_probe() << std::endl;
    std::cout << "Ibo_probe =" << getIbo_probe() << std::endl;

    for (int i = 0; i < 139; i++)
      {
	if (i%8==0) printf("\n");
	printf("%.4x ", _l[i]);
      }
  }

  ///  store in JSON SLC values
  void toJson()
  {
    //_jasic.clear();
    web::json::value _kasic;
    _kasic["EN_pa"]=getEN_pa();
    _kasic["PP_pa"]=getPP_pa();
    _kasic["PA_gain"]=getPA_gain();
    _kasic["EN_7b"]=getEN_7b();
    _kasic["PP_7b"]=getPP_7b();
    _kasic["EN_disc"]=getEN_disc();
    _kasic["PP_disc"]=getPP_disc();
    _kasic["Polarity"]=getPolarity();
    _kasic["Hysteresys"]=getHysteresys();
    _kasic["EN_bg"]=getEN_bg();
    _kasic["PP_bg"]=getPP_bg();
    _kasic["EN_10bDAC"]=getEN_10bDAC();
    _kasic["PP_10bDAC"]=getPP_10bDAC();
    _kasic["DAC_threshold"]=getDAC_threshold();

    _kasic["CLPS_bsize"]=getCLPS_bsize();
    _kasic["EN_pre_emphasis"]=getEN_pre_emphasis();
    _kasic["Pre_emphasis_delay"]=getPre_emphasis_delay();
    _kasic["EN_differential"]=getEN_differential();
    _kasic["PP_differential"]=getPP_differential();
    _kasic["Forced_ValEvt"]=getForced_ValEvt();
    _kasic["EN_probe"]=getEN_probe();
    _kasic["PP_probe"]=getPP_probe();
    _kasic["MillerComp"]=getMillerComp();
    _kasic["Ibi_probe"]=getIbi_probe();
    _kasic["Ibo_probe"]=getIbo_probe();

    web::json::value dc_pa;
    web::json::value ctest;
    web::json::value mask;
    web::json::value dac_local;
    for (int ch = 0; ch < 64; ch++)
      {
	dc_pa[ch]=((int)getDC_pa(ch));
	ctest[ch]=((int)getCtest(ch));
	mask[ch]=((int)getMask(ch));
	dac_local[ch]=((int)getDAC_local(ch));

      }
    _kasic["DC_pa"] = dc_pa;
    _kasic["Ctest"] = ctest;
    _kasic["Mask"] = mask;
    _kasic["DAC_local"] = dac_local;
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


  uint32_t *board_ptr() {
    for (int i=0;i<139;i++)
      {
	uint8_t address,sub;
	if (i<128)
	  {
	    address=i/2;
	    sub=i%2;
	  }
	else
	  {
	    int j=i-128;
	    address=64+j/3;
	    sub=j%3;
	  }
	_board[i]=_l[i]|(adress<<8)|(sub<<16);
      }
    return _board;
  }
  /**
   * \brief Setters and getters (see PETIROC Manuals for details)
   * */

  /// Ivert bit orders
  void loadInvert(LRSlow r)
  {

    for (int i = 1112; i >= 0; i--)
      setBitState(1112 - i, r.getBit(i) == 1);
  }

  /// Fill address & value for FEB slow control
  void prepareBoard()
  {
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
  uint8_t _l[139]; ///< 640 bits of SLC
  web::json::value _jasic; ///< JSON cpp Value 
};
