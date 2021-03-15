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
class HR2Slow
  {
  public:
    HR2Slow(){ memset(_l,0,28*sizeof(uint32_t));}
    bool getBit(uint32_t* l,int b){return (l[b/32]>>(b%32))&1;}
    void setBit(uint32_t* l,int b){ l[b/32]|=(1<<(b%32));};
    void clearBit(uint32_t* l,int b){ l[b/32]&= ~(1<<(b%32));};
    void setBitState(uint32_t* l,int b,bool t){if (t) setBit(l,b); else clearBit(l,b);}
    
    bool getBit(int b){return (_l[b/32]>>(b%32))&1;}
    void setBit(int b){ _l[b/32]|=(1<<(b%32));};
    void clearBit(int b){ _l[b/32]&= ~(1<<(b%32));};
    void setBitState(int b,bool t){if (t) setBit(b); else clearBit(b);}
    uint8_t getByte(int b){return
	(getBit(b)|getBit(b+1)<<1|getBit(b+2)<<2|getBit(b+3)<<3|getBit(b+4)<<4|getBit(b+5)<<5|getBit(b+6)<<6|getBit(b+7)<<7);
    }
    void setByte(int b,uint8_t val,uint8_t len)
    {
      //unsigned char* p=&_l[b/32];
      for(int i=0;i<len;i++)
	if ((val>>i)&1)
	  setBit(b+i);
	else
	  clearBit(b+i);
    }
    void Print()
    {
      for (int i=0;i<28;i++)
	printf("%d %x \n",i,_l[i]);
      /* getchar();
       */


    
    }
    void toJson()
    {
      web::json::value _jjasic;

      _jjasic["ENABLED"]=1;
      _jjasic["EN_OTAQ"]=getEN_OTAQ();
      _jjasic["DACSW"]=getDACSW();
      _jjasic["SEL0"]=getSEL0();
      _jjasic["CMDB2FSB2"]=getCMDB2FSB2();
      _jjasic["CMDB0FSB2"]=getCMDB0FSB2();
      _jjasic["ENOCTRANSMITON1B"]=getENOCTRANSMITON1B();
      _jjasic["SEL1"]=getSEL1();
      _jjasic["CMDB2FSB1"]=getCMDB2FSB1();
      _jjasic["OTABGSW"]=getOTABGSW();
      _jjasic["SW50F0"]=getSW50F0();
      _jjasic["ENOCDOUT2B"]=getENOCDOUT2B();
      _jjasic["SW50K0"]=getSW50K0();
      _jjasic["SMALLDAC"]=getSMALLDAC();
      _jjasic["SELENDREADOUT"]=getSELENDREADOUT();
      _jjasic["CMDB3FSB1"]=getCMDB3FSB1();
      _jjasic["PWRONBUFF"]=getPWRONBUFF();
      _jjasic["ENOCDOUT1B"]=getENOCDOUT1B();
      _jjasic["SW50K2"]=getSW50K2();
      _jjasic["CMDB2SS"]=getCMDB2SS();
      _jjasic["TRIG1B"]=getTRIG1B();
      _jjasic["CMDB1SS"]=getCMDB1SS();
      _jjasic["PWRONPA"]=getPWRONPA();
      _jjasic["PWRONSS"]=getPWRONSS();
      _jjasic["RAZCHNINTVAL"]=getRAZCHNINTVAL();
      _jjasic["SW100K1"]=getSW100K1();
      _jjasic["CLKMUX"]=getCLKMUX();
      _jjasic["SW50F1"]=getSW50F1();
      _jjasic["PWRONFSB0"]=getPWRONFSB0();
      _jjasic["ENOCTRANSMITON2B"]=getENOCTRANSMITON2B();
      _jjasic["ENOCCHIPSATB"]=getENOCCHIPSATB();
      _jjasic["CMDB3SS"]=getCMDB3SS();
      _jjasic["DISCRI1"]=getDISCRI1();
      _jjasic["SW50F2"]=getSW50F2();
      _jjasic["SW100K0"]=getSW100K0();
      _jjasic["CMDB3FSB2"]=getCMDB3FSB2();
      _jjasic["SCON"]=getSCON();
      _jjasic["TRIG2B"]=getTRIG2B();
      _jjasic["SW100K2"]=getSW100K2();
      _jjasic["SW100F1"]=getSW100F1();
      _jjasic["TRIGEXTVAL"]=getTRIGEXTVAL();
      _jjasic["PWRONFSB2"]=getPWRONFSB2();
      _jjasic["RS_OR_DISCRI"]=getRS_OR_DISCRI();
      _jjasic["TRIG0B"]=getTRIG0B();
      _jjasic["PWRONFSB1"]=getPWRONFSB1();
      _jjasic["SW100F2"]=getSW100F2();
      _jjasic["PWRONW"]=getPWRONW();
      _jjasic["RAZCHNEXTVAL"]=getRAZCHNEXTVAL();
      _jjasic["DISCRI2"]=getDISCRI2();
      _jjasic["SELSTARTREADOUT"]=getSELSTARTREADOUT();
      _jjasic["SW50K1"]=getSW50K1();
      _jjasic["CMDB1FSB1"]=getCMDB1FSB1();
      _jjasic["ENTRIGOUT"]=getENTRIGOUT();
      _jjasic["CMDB0SS"]=getCMDB0SS();
      _jjasic["SW100F0"]=getSW100F0();
      _jjasic["CMDB1FSB2"]=getCMDB1FSB2();
      _jjasic["OTAQ_PWRADC"]=getOTAQ_PWRADC();
      _jjasic["CMDB0FSB1"]=getCMDB0FSB1();
      _jjasic["DISCROROR"]=getDISCROROR();
      _jjasic["DISCRI0"]=getDISCRI0();
      _jjasic["B0"]=getB0();
      _jjasic["B1"]=getB1();
      _jjasic["B2"]=getB2();
      _jjasic["HEADER"]=getHEADER();


      json::value ctest;
      json::value m0; json::value m1;json::value m2; json::value pag;
      for (int ch=0;ch<64;ch++)
	{
	  ctest[ch]=((int) getCTEST(ch));
	  pag[ch]=((int) getPAGAIN(ch));
	  m0[ch]=((int) getMASKChannel(0,ch));
	  m1[ch]=((int) getMASKChannel(1,ch));
	  m2[ch]=((int) getMASKChannel(2,ch));
	}
      _jjasic["CTEST"]=ctest;
      _jjasic["MASK0"]=m0;
      _jjasic["MASK1"]=m1;
      _jjasic["MASK2"]=m2;
      _jjasic["PAGAIN"]=pag;
   
      _jasic=_jjasic;
    }
    void dumpJson()
    {
     
      std::cout <<_jasic << std::endl;
    }
    void dumpBinary()
    {
      uint8_t* b=(uint8_t*) _l;
      for (int i=108;i>=0;i--)
	fprintf(stderr,"%.2x",b[i]);
      fprintf(stderr,"\n");
    }
    
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
      return;



      
    }
    void setFromJson()
    {
    
      

      uint8_t ch=0;
      for (auto it = _jasic["PAGAIN"].as_array().begin(); it !=_jasic["PAGAIN"].as_array().end(); it++)
	{
	  uint8_t sid=(*it).as_integer();
	  setPAGAIN(ch,sid);
	  ch++;
	}
      ch=0;
      for (auto it = _jasic["CTEST"].as_array().begin(); it !=_jasic["CTEST"].as_array().end(); it++)
	{
	  uint8_t sid=(*it).as_integer();
	  setCTEST(ch,sid);
	  ch++;
	}
      ch=0;
      for (auto it = _jasic["MASK0"].as_array().begin(); it !=_jasic["MASK0"].as_array().end(); it++)
	{
	  uint8_t sid=(*it).as_integer();
	  setMASKChannel(0,ch,sid);
	  ch++;
	}
      ch=0;
      for (auto it = _jasic["MASK1"].as_array().begin(); it !=_jasic["MASK1"].as_array().end(); it++)
	{
	  uint8_t sid=(*it).as_integer();
	  setMASKChannel(1,ch,sid);
	  ch++;
	}
      ch=0;
      for (auto it = _jasic["MASK2"].as_array().begin(); it !=_jasic["MASK2"].as_array().end(); it++)
	{
	  uint8_t sid=(*it).as_integer();
	  setMASKChannel(2,ch,sid);
	  ch++;
	}

      
      setEN_OTAQ(_jasic["EN_OTAQ"].as_integer());
      setDACSW(_jasic["DACSW"].as_integer());
      setSEL0(_jasic["SEL0"].as_integer());
      setCMDB2FSB2(_jasic["CMDB2FSB2"].as_integer());
      setCMDB0FSB2(_jasic["CMDB0FSB2"].as_integer());
      setENOCTRANSMITON1B(_jasic["ENOCTRANSMITON1B"].as_integer());
      setSEL1(_jasic["SEL1"].as_integer());
      setCMDB2FSB1(_jasic["CMDB2FSB1"].as_integer());
      setOTABGSW(_jasic["OTABGSW"].as_integer());
      setSW50F0(_jasic["SW50F0"].as_integer());
      setENOCDOUT2B(_jasic["ENOCDOUT2B"].as_integer());
      setSW50K0(_jasic["SW50K0"].as_integer());
      setSMALLDAC(_jasic["SMALLDAC"].as_integer());
      setSELENDREADOUT(_jasic["SELENDREADOUT"].as_integer());
      setCMDB3FSB1(_jasic["CMDB3FSB1"].as_integer());
      setSWSSC(_jasic["SWSSC"].as_integer());
      setPWRONBUFF(_jasic["PWRONBUFF"].as_integer());
      setENOCDOUT1B(_jasic["ENOCDOUT1B"].as_integer());
      setSW50K2(_jasic["SW50K2"].as_integer());
      setCMDB2SS(_jasic["CMDB2SS"].as_integer());
      setTRIG1B(_jasic["TRIG1B"].as_integer());
      setCMDB1SS(_jasic["CMDB1SS"].as_integer());
      setPWRONPA(_jasic["PWRONPA"].as_integer());
      setPWRONSS(_jasic["PWRONSS"].as_integer());
      setRAZCHNINTVAL(_jasic["RAZCHNINTVAL"].as_integer());
      setSW100K1(_jasic["SW100K1"].as_integer());
      setCLKMUX(_jasic["CLKMUX"].as_integer());
      setSW50F1(_jasic["SW50F1"].as_integer());
      setPWRONFSB0(_jasic["PWRONFSB0"].as_integer());
      setENOCTRANSMITON2B(_jasic["ENOCTRANSMITON2B"].as_integer());
      setENOCCHIPSATB(_jasic["ENOCCHIPSATB"].as_integer());
      setCMDB3SS(_jasic["CMDB3SS"].as_integer());
      setDISCRI1(_jasic["DISCRI1"].as_integer());
      setSW50F2(_jasic["SW50F2"].as_integer());
      setSW100K0(_jasic["SW100K0"].as_integer());
      setCMDB3FSB2(_jasic["CMDB3FSB2"].as_integer());
      setSCON(_jasic["SCON"].as_integer());
      setTRIG2B(_jasic["TRIG2B"].as_integer());
      setSW100K2(_jasic["SW100K2"].as_integer());
      setSW100F1(_jasic["SW100F1"].as_integer());
      setTRIGEXTVAL(_jasic["TRIGEXTVAL"].as_integer());
      setPWRONFSB2(_jasic["PWRONFSB2"].as_integer());
      setRS_OR_DISCRI(_jasic["RS_OR_DISCRI"].as_integer());
      setTRIG0B(_jasic["TRIG0B"].as_integer());
      setPWRONFSB1(_jasic["PWRONFSB1"].as_integer());
      setSW100F2(_jasic["SW100F2"].as_integer());
      setPWRONW(_jasic["PWRONW"].as_integer());
      setRAZCHNEXTVAL(_jasic["RAZCHNEXTVAL"].as_integer());
      setDISCRI2(_jasic["DISCRI2"].as_integer());
      setSELSTARTREADOUT(_jasic["SELSTARTREADOUT"].as_integer());
      setSW50K1(_jasic["SW50K1"].as_integer());
      setCMDB1FSB1(_jasic["CMDB1FSB1"].as_integer());
      setENTRIGOUT(_jasic["ENTRIGOUT"].as_integer());
      setCMDB0SS(_jasic["CMDB0SS"].as_integer());
      setSW100F0(_jasic["SW100F0"].as_integer());
      setCMDB1FSB2(_jasic["CMDB1FSB2"].as_integer());
      setOTAQ_PWRADC(_jasic["OTAQ_PWRADC"].as_integer());
      setCMDB0FSB1(_jasic["CMDB0FSB1"].as_integer());
      setDISCROROR(_jasic["DISCROROR"].as_integer());
      setDISCRI0(_jasic["DISCRI0"].as_integer());

      setB0(_jasic["B0"].as_integer());
      setB1(_jasic["B1"].as_integer());
      setB2(_jasic["B2"].as_integer());

      std::cout<<" ASIC HEADER "<<_jasic["HEADER"].as_integer()<<std::endl;
      setHEADER(_jasic["HEADER"].as_integer());
      std::cout<<"GET HEADER "<<(int) getHEADER()<<std::endl;

    
    }
   
    uint32_t* ptr(){return _l;}
    uint8_t* ucPtr(){return (uint8_t*) _l;}
    // Now real access to register function
    
    uint16_t getB2()
    {
      uint16_t r=0;
      for (int i=0;i<10;i++)
	if (getBit(838+i)) r|=(1<<i);
      return r;
    }
    void setB2(uint16_t val)
    {
      uint16_t r=val&0x3FF;
      for (int i=0;i<10;i++)
	setBitState(838+i,r&(1<<i));
    }

    uint16_t getB1()
    {
      uint16_t r=0;
      for (int i=0;i<10;i++)
	if (getBit(828+i)) r|=(1<<i);
      return r;
    }
    void setB1(uint16_t val)
    {
      uint16_t r=val&0x3FF;
      for (int i=0;i<10;i++)
	setBitState(828+i,r&(1<<i));
    }
    uint16_t getB0()
    {
      uint16_t r=0;
      for (int i=0;i<10;i++)
	if (getBit(818+i)) r|=(1<<i);
      return r;
    }
    void setB0(uint16_t val)
    {
      uint16_t r=val&0x3FF;
      for (int i=0;i<10;i++)
	setBitState(818+i,r&(1<<i));
    }
    uint8_t getHEADER(){return getByte(810);}
#define HEADER_INVERTED
#ifndef HEADER_INVERTED
    void setHEADER(uint8_t val){setByte(810,val,8);}
#else
     void setHEADER(uint16_t val)
    {
      uint16_t r=val&0xFF;
      for (int i=0;i<8;i++)
	setBitState(817-i,r&(1<<i));
    }
 #endif
    uint64_t getMASK(uint8_t level)
    {
      uint64_t mask=0;
      for (int i=0;i<64;i++)
	{
	  if (getBit(618+i*3+level)==1)
	    mask|=(1<<i);
	}
      return mask;
    }
    void setMASK(uint8_t level,uint64_t mask)
    {
      for (int i=0;i<64;i++)
	{
	  setBitState(618+i*3+level, ((mask>>i)&1)!=0);
	}
    }
    bool getMASKChannel(uint8_t level,int ch)
    {
      return getBit(618+(ch&63)*3+level);
    }
    void setMASKChannel(uint8_t level,int ch,bool on)
    {
      int i=ch&63;
      setBitState(618+i*3+level,on);
	}


    uint8_t getSWSSC(){return getByte(581)&0x7;}
    void setSWSSC(uint8_t val){setByte(581,val&0x7,3);}

    uint8_t  getPAGAIN(int ch){return getByte(64+8*ch);}
    void setPAGAIN(int ch,uint8_t val){setByte(64+8*ch,val,8);}
    
    bool getCTEST(int b){return getBit(b%64);}
    void setCTEST(int b,bool on)  { setBitState(b%64,on);}


    bool getEN_OTAQ(){return getBit(612);}
    void setEN_OTAQ(bool on){setBitState(612,on);}
    bool getDACSW(){return getBit(849);}
    void setDACSW(bool on){setBitState(849,on);}
    bool getSEL0(){return getBit(603);}
    void setSEL0(bool on){setBitState(603,on);}
    bool getCMDB2FSB2(){return getBit(588);}
    void setCMDB2FSB2(bool on){setBitState(588,on);}
    bool getCMDB0FSB2(){return getBit(590);}
    void setCMDB0FSB2(bool on){setBitState(590,on);}
    bool getENOCTRANSMITON1B(){return getBit(869);}
    void setENOCTRANSMITON1B(bool on){setBitState(869,on);}
    bool getSEL1(){return getBit(604);}
    void setSEL1(bool on){setBitState(604,on);}
    bool getCMDB2FSB1(){return getBit(596);}
    void setCMDB2FSB1(bool on){setBitState(596,on);}
    bool getOTABGSW(){return getBit(850);}
    void setOTABGSW(bool on){setBitState(850,on);}
    bool getSW50F0(){return getBit(611);}
    void setSW50F0(bool on){setBitState(611,on);}
    bool getENOCDOUT2B(){return getBit(870);}
    void setENOCDOUT2B(bool on){setBitState(870,on);}
    bool getSW50K0(){return getBit(608);}
    void setSW50K0(bool on){setBitState(608,on);}
    bool getSMALLDAC(){return getBit(848);}
    void setSMALLDAC(bool on){setBitState(848,on);}
    bool getSELENDREADOUT(){return getBit(865);}
    void setSELENDREADOUT(bool on){setBitState(865,on);}
    bool getCMDB3FSB1(){return getBit(595);}
    void setCMDB3FSB1(bool on){setBitState(595,on);}
    bool getPWRONBUFF(){return getBit(584);}
    void setPWRONBUFF(bool on){setBitState(584,on);}
    bool getENOCDOUT1B(){return getBit(871);}
    void setENOCDOUT1B(bool on){setBitState(871,on);}
    bool getSW50K2(){return getBit(591);}
    void setSW50K2(bool on){setBitState(591,on);}
    bool getCMDB2SS(){return getBit(578);}
    void setCMDB2SS(bool on){setBitState(578,on);}
    bool getTRIG1B(){return getBit(852);}
    void setTRIG1B(bool on){setBitState(852,on);}
    bool getCMDB1SS(){return getBit(579);}
    void setCMDB1SS(bool on){setBitState(579,on);}
    bool getPWRONPA(){return getBit(576);}
    void setPWRONPA(bool on){setBitState(576,on);}
    bool getPWRONSS(){return getBit(585);}
    void setPWRONSS(bool on){setBitState(585,on);}
    bool getRAZCHNINTVAL(){return getBit(857);}
    void setRAZCHNINTVAL(bool on){setBitState(857,on);}
    bool getSW100K1(){return getBit(600);}
    void setSW100K1(bool on){setBitState(600,on);}
    bool getCLKMUX(){return getBit(860);}
    void setCLKMUX(bool on){setBitState(860,on);}
    bool getSW50F1(){return getBit(602);}
    void setSW50F1(bool on){setBitState(602,on);}
    bool getPWRONFSB0(){return getBit(605);}
    void setPWRONFSB0(bool on){setBitState(605,on);}
    bool getENOCTRANSMITON2B(){return getBit(868);}
    void setENOCTRANSMITON2B(bool on){setBitState(868,on);}
    bool getENOCCHIPSATB(){return getBit(867);}
    void setENOCCHIPSATB(bool on){setBitState(867,on);}
    bool getCMDB3SS(){return getBit(577);}
    void setCMDB3SS(bool on){setBitState(577,on);}
    bool getDISCRI1(){return getBit(616);}
    void setDISCRI1(bool on){setBitState(616,on);}
    bool getSW50F2(){return getBit(594);}
    void setSW50F2(bool on){setBitState(594,on);}
    bool getSW100K0(){return getBit(609);}
    void setSW100K0(bool on){setBitState(609,on);}
    bool getCMDB3FSB2(){return getBit(587);}
    void setCMDB3FSB2(bool on){setBitState(587,on);}
    bool getSCON(){return getBit(859);}
    void setSCON(bool on){setBitState(859,on);}
    bool getTRIG2B(){return getBit(851);}
    void setTRIG2B(bool on){setBitState(851,on);}
    bool getSW100K2(){return getBit(592);}
    void setSW100K2(bool on){setBitState(592,on);}
    bool getSW100F1(){return getBit(601);}
    void setSW100F1(bool on){setBitState(601,on);}
    bool getTRIGEXTVAL(){return getBit(856);}
    void setTRIGEXTVAL(bool on){setBitState(856,on);}
    bool getPWRONFSB2(){return getBit(606);}
    void setPWRONFSB2(bool on){setBitState(606,on);}
    bool getRS_OR_DISCRI(){return getBit(617);}
    void setRS_OR_DISCRI(bool on){setBitState(617,on);}
    bool getTRIG0B(){return getBit(853);}
    void setTRIG0B(bool on){setBitState(853,on);}
    bool getPWRONFSB1(){return getBit(607);}
    void setPWRONFSB1(bool on){setBitState(607,on);}
    bool getSW100F2(){return getBit(593);}
    void setSW100F2(bool on){setBitState(593,on);}
    bool getPWRONW(){return getBit(586);}
    void setPWRONW(bool on){setBitState(586,on);}
    bool getRAZCHNEXTVAL(){return getBit(858);}
    void setRAZCHNEXTVAL(bool on){setBitState(858,on);}
    bool getDISCRI2(){return getBit(615);}
    void setDISCRI2(bool on){setBitState(615,on);}
    bool getSELSTARTREADOUT(){return getBit(866);}
    void setSELSTARTREADOUT(bool on){setBitState(866,on);}
    bool getSW50K1(){return getBit(599);}
    void setSW50K1(bool on){setBitState(599,on);}
    bool getCMDB1FSB1(){return getBit(597);}
    void setCMDB1FSB1(bool on){setBitState(597,on);}
    bool getENTRIGOUT(){return getBit(854);}
    void setENTRIGOUT(bool on){setBitState(854,on);}
    bool getCMDB0SS(){return getBit(580);}
    void setCMDB0SS(bool on){setBitState(580,on);}
    bool getSW100F0(){return getBit(610);}
    void setSW100F0(bool on){setBitState(610,on);}
    bool getCMDB1FSB2(){return getBit(589);}
    void setCMDB1FSB2(bool on){setBitState(589,on);}
    bool getOTAQ_PWRADC(){return getBit(613);}
    void setOTAQ_PWRADC(bool on){setBitState(613,on);}
    bool getCMDB0FSB1(){return getBit(598);}
    void setCMDB0FSB1(bool on){setBitState(598,on);}
    bool getDISCROROR(){return getBit(855);}
    void setDISCROROR(bool on){setBitState(855,on);}
    bool getDISCRI0(){return getBit(614);}
    void setDISCRI0(bool on){setBitState(614,on);}

    bool isEnabled()
    {

      if (_jasic.as_object().find("ENABLED")!=_jasic.as_object().end())

	return (_jasic["ENABLED"]==1);
      else
	return true;
    }
    void setEnable(bool i) {_jasic["ENABLED"]=i;}
    void invertBits(HR2Slow* r)
    {

      for (int i=871;i>=0;i--)
	r->setBitState(871-i,getBit(i)==1);

    }

    uint8_t* ucInvertedPtr()
    {

      for (int i=871;i>=0;i--)
	setBitState(_li,871-i,getBit(_l,i)==1);
      
      return (uint8_t*) _li;
    }
 
    web::json::value& getJson(){return _jasic;}
    void setJson(web::json::value v){_jasic=v; this->setFromJson();}

    
  private:
    uint32_t _l[28];
    uint32_t _li[28];
    web::json::value _jasic;
  };
