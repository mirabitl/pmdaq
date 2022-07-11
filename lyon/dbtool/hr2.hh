#pragma once
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
namespace lmdbtool
{
  class hr2
  {
  public:
    hr2() { memset(_l, 0, 28 * sizeof(uint32_t)); }
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
    void toJson()
    {
      _jasic.clear();
      _jasic["ENABLED"] = 1;
      _jasic["EN_OTAQ"] = getEN_OTAQ();
      _jasic["DACSW"] = getDACSW();
      _jasic["SEL0"] = getSEL0();
      _jasic["CMDB2FSB2"] = getCMDB2FSB2();
      _jasic["CMDB0FSB2"] = getCMDB0FSB2();
      _jasic["ENOCTRANSMITON1B"] = getENOCTRANSMITON1B();
      _jasic["SEL1"] = getSEL1();
      _jasic["CMDB2FSB1"] = getCMDB2FSB1();
      _jasic["OTABGSW"] = getOTABGSW();
      _jasic["SW50F0"] = getSW50F0();
      _jasic["ENOCDOUT2B"] = getENOCDOUT2B();
      _jasic["SW50K0"] = getSW50K0();
      _jasic["SMALLDAC"] = getSMALLDAC();
      _jasic["SELENDREADOUT"] = getSELENDREADOUT();
      _jasic["CMDB3FSB1"] = getCMDB3FSB1();
      _jasic["PWRONBUFF"] = getPWRONBUFF();
      _jasic["ENOCDOUT1B"] = getENOCDOUT1B();
      _jasic["SW50K2"] = getSW50K2();
      _jasic["CMDB2SS"] = getCMDB2SS();
      _jasic["TRIG1B"] = getTRIG1B();
      _jasic["CMDB1SS"] = getCMDB1SS();
      _jasic["PWRONPA"] = getPWRONPA();
      _jasic["PWRONSS"] = getPWRONSS();
      _jasic["RAZCHNINTVAL"] = getRAZCHNINTVAL();
      _jasic["SW100K1"] = getSW100K1();
      _jasic["CLKMUX"] = getCLKMUX();
      _jasic["SW50F1"] = getSW50F1();
      _jasic["PWRONFSB0"] = getPWRONFSB0();
      _jasic["ENOCTRANSMITON2B"] = getENOCTRANSMITON2B();
      _jasic["ENOCCHIPSATB"] = getENOCCHIPSATB();
      _jasic["CMDB3SS"] = getCMDB3SS();
      _jasic["DISCRI1"] = getDISCRI1();
      _jasic["SW50F2"] = getSW50F2();
      _jasic["SW100K0"] = getSW100K0();
      _jasic["CMDB3FSB2"] = getCMDB3FSB2();
      _jasic["SCON"] = getSCON();
      _jasic["TRIG2B"] = getTRIG2B();
      _jasic["SW100K2"] = getSW100K2();
      _jasic["SW100F1"] = getSW100F1();
      _jasic["TRIGEXTVAL"] = getTRIGEXTVAL();
      _jasic["PWRONFSB2"] = getPWRONFSB2();
      _jasic["RS_OR_DISCRI"] = getRS_OR_DISCRI();
      _jasic["TRIG0B"] = getTRIG0B();
      _jasic["PWRONFSB1"] = getPWRONFSB1();
      _jasic["SW100F2"] = getSW100F2();
      _jasic["PWRONW"] = getPWRONW();
      _jasic["RAZCHNEXTVAL"] = getRAZCHNEXTVAL();
      _jasic["DISCRI2"] = getDISCRI2();
      _jasic["SELSTARTREADOUT"] = getSELSTARTREADOUT();
      _jasic["SW50K1"] = getSW50K1();
      _jasic["CMDB1FSB1"] = getCMDB1FSB1();
      _jasic["ENTRIGOUT"] = getENTRIGOUT();
      _jasic["CMDB0SS"] = getCMDB0SS();
      _jasic["SW100F0"] = getSW100F0();
      _jasic["CMDB1FSB2"] = getCMDB1FSB2();
      _jasic["OTAQ_PWRADC"] = getOTAQ_PWRADC();
      _jasic["CMDB0FSB1"] = getCMDB0FSB1();
      _jasic["DISCROROR"] = getDISCROROR();
      _jasic["DISCRI0"] = getDISCRI0();
      _jasic["B0"] = getB0();
      _jasic["B1"] = getB1();
      _jasic["B2"] = getB2();
      _jasic["HEADER"] = getHEADER();

      Json::Value ctest;
      Json::Value m0;
      Json::Value m1;
      Json::Value m2;
      Json::Value pag;
      for (int ch = 0; ch < 64; ch++)
      {
        ctest.append((int)getCTEST(ch));
        pag.append((int)getPAGAIN(ch));
        m0.append((int)getMASKChannel(0, ch));
        m1.append((int)getMASKChannel(1, ch));
        m2.append((int)getMASKChannel(2, ch));
      }
      _jasic["CTEST"] = ctest;
      _jasic["MASK0"] = m0;
      _jasic["MASK1"] = m1;
      _jasic["MASK2"] = m2;
      _jasic["PAGAIN"] = pag;
    }
    void dumpJson()
    {
      Json::StyledWriter styledWriter;
      std::cout << styledWriter.write(_jasic) << std::endl;
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
      Json::Reader reader;
      // bool parsingSuccessful = reader.parse(fname,_jasic);

      std::ifstream ifs(fname.c_str(), std::ifstream::in);

      bool parsingSuccessful = reader.parse(ifs, _jasic, false);

      if (parsingSuccessful)
      {
        Json::StyledWriter styledWriter;
        // std::cout << styledWriter.write(_jasic) << std::endl;
      }
      else
        std::cout << "parsing erro " << std::endl;
    }
    void setFromJson()
    {
      const Json::Value &books = _jasic["PAGAIN"];
      uint8_t ch = 0;
      for (Json::ValueConstIterator it = books.begin(); it != books.end(); ++it)
      {
        const Json::Value &book = *it;
        int isid = (*it).asInt();
        uint8_t sid= isid & 0xFF;
        setPAGAIN(ch, sid);
        ch++;
      }
      const Json::Value &books1 = _jasic["CTEST"];
      ch = 0;
      for (Json::ValueConstIterator it = books1.begin(); it != books1.end(); ++it)
      {
        const Json::Value &book = *it;
        uint8_t sid = (*it).asUInt();
        setCTEST(ch, sid);
        ch++;
      }
      const Json::Value &books2 = _jasic["MASK0"];
      ch = 0;
      for (Json::ValueConstIterator it = books2.begin(); it != books2.end(); ++it)
      {
        const Json::Value &book = *it;
        uint8_t sid = (*it).asUInt();
        setMASKChannel(0, ch, sid == 1);
        ch++;
      }
      const Json::Value &books3 = _jasic["MASK1"];
      ch = 0;
      for (Json::ValueConstIterator it = books3.begin(); it != books3.end(); ++it)
      {
        const Json::Value &book = *it;
        uint8_t sid = (*it).asUInt();
        setMASKChannel(1, ch, sid == 1);
        ch++;
      }
      const Json::Value &books4 = _jasic["MASK2"];
      ch = 0;
      for (Json::ValueConstIterator it = books4.begin(); it != books4.end(); ++it)
      {
        const Json::Value &book = *it;
        uint8_t sid = (*it).asUInt();
        setMASKChannel(2, ch, sid == 1);
        ch++;
      }

      setEN_OTAQ(_jasic["EN_OTAQ"].asUInt());
      setDACSW(_jasic["DACSW"].asUInt());
      setSEL0(_jasic["SEL0"].asUInt());
      setCMDB2FSB2(_jasic["CMDB2FSB2"].asUInt());
      setCMDB0FSB2(_jasic["CMDB0FSB2"].asUInt());
      setENOCTRANSMITON1B(_jasic["ENOCTRANSMITON1B"].asUInt());
      setSEL1(_jasic["SEL1"].asUInt());
      setCMDB2FSB1(_jasic["CMDB2FSB1"].asUInt());
      setOTABGSW(_jasic["OTABGSW"].asUInt());
      setSW50F0(_jasic["SW50F0"].asUInt());
      setENOCDOUT2B(_jasic["ENOCDOUT2B"].asUInt());
      setSW50K0(_jasic["SW50K0"].asUInt());
      setSMALLDAC(_jasic["SMALLDAC"].asUInt());
      setSELENDREADOUT(_jasic["SELENDREADOUT"].asUInt());
      setCMDB3FSB1(_jasic["CMDB3FSB1"].asUInt());
      setSWSSC(_jasic["SWSSC"].asUInt());
      setPWRONBUFF(_jasic["PWRONBUFF"].asUInt());
      setENOCDOUT1B(_jasic["ENOCDOUT1B"].asUInt());
      setSW50K2(_jasic["SW50K2"].asUInt());
      setCMDB2SS(_jasic["CMDB2SS"].asUInt());
      setTRIG1B(_jasic["TRIG1B"].asUInt());
      setCMDB1SS(_jasic["CMDB1SS"].asUInt());
      setPWRONPA(_jasic["PWRONPA"].asUInt());
      setPWRONSS(_jasic["PWRONSS"].asUInt());
      setRAZCHNINTVAL(_jasic["RAZCHNINTVAL"].asUInt());
      setSW100K1(_jasic["SW100K1"].asUInt());
      setCLKMUX(_jasic["CLKMUX"].asUInt());
      setSW50F1(_jasic["SW50F1"].asUInt());
      setPWRONFSB0(_jasic["PWRONFSB0"].asUInt());
      setENOCTRANSMITON2B(_jasic["ENOCTRANSMITON2B"].asUInt());
      setENOCCHIPSATB(_jasic["ENOCCHIPSATB"].asUInt());
      setCMDB3SS(_jasic["CMDB3SS"].asUInt());
      setDISCRI1(_jasic["DISCRI1"].asUInt());
      setSW50F2(_jasic["SW50F2"].asUInt());
      setSW100K0(_jasic["SW100K0"].asUInt());
      setCMDB3FSB2(_jasic["CMDB3FSB2"].asUInt());
      setSCON(_jasic["SCON"].asUInt());
      setTRIG2B(_jasic["TRIG2B"].asUInt());
      setSW100K2(_jasic["SW100K2"].asUInt());
      setSW100F1(_jasic["SW100F1"].asUInt());
      setTRIGEXTVAL(_jasic["TRIGEXTVAL"].asUInt());
      setPWRONFSB2(_jasic["PWRONFSB2"].asUInt());
      setRS_OR_DISCRI(_jasic["RS_OR_DISCRI"].asUInt());
      setTRIG0B(_jasic["TRIG0B"].asUInt());
      setPWRONFSB1(_jasic["PWRONFSB1"].asUInt());
      setSW100F2(_jasic["SW100F2"].asUInt());
      setPWRONW(_jasic["PWRONW"].asUInt());
      setRAZCHNEXTVAL(_jasic["RAZCHNEXTVAL"].asUInt());
      setDISCRI2(_jasic["DISCRI2"].asUInt());
      setSELSTARTREADOUT(_jasic["SELSTARTREADOUT"].asUInt());
      setSW50K1(_jasic["SW50K1"].asUInt());
      setCMDB1FSB1(_jasic["CMDB1FSB1"].asUInt());
      setENTRIGOUT(_jasic["ENTRIGOUT"].asUInt());
      setCMDB0SS(_jasic["CMDB0SS"].asUInt());
      setSW100F0(_jasic["SW100F0"].asUInt());
      setCMDB1FSB2(_jasic["CMDB1FSB2"].asUInt());
      setOTAQ_PWRADC(_jasic["OTAQ_PWRADC"].asUInt());
      setCMDB0FSB1(_jasic["CMDB0FSB1"].asUInt());
      setDISCROROR(_jasic["DISCROROR"].asUInt());
      setDISCRI0(_jasic["DISCRI0"].asUInt());

      setB0(_jasic["B0"].asUInt());
      setB1(_jasic["B1"].asUInt());
      setB2(_jasic["B2"].asUInt());

      //std::cout << " ASIC HEADER " << _jasic["HEADER"].asUInt() << std::endl;
      setHEADER(_jasic["HEADER"].asUInt());
      //std::cout << "GET HEADER " << (int)getHEADER() << std::endl;
    }

    uint32_t *ptr() { return _l; }
    uint8_t *ucPtr() { return (uint8_t *)_l; }
    // Now real access to register function

    uint16_t getB2()
    {
      uint16_t r = 0;
      for (int i = 0; i < 10; i++)
        if (getBit(838 + i))
          r |= (1 << i);
      return r;
    }
    void setB2(uint16_t val)
    {
      uint16_t r = val & 0x3FF;
      for (int i = 0; i < 10; i++)
        setBitState(838 + i, r & (1 << i));
    }

    uint16_t getB1()
    {
      uint16_t r = 0;
      for (int i = 0; i < 10; i++)
        if (getBit(828 + i))
          r |= (1 << i);
      return r;
    }
    void setB1(uint16_t val)
    {
      uint16_t r = val & 0x3FF;
      for (int i = 0; i < 10; i++)
        setBitState(828 + i, r & (1 << i));
    }
    uint16_t getB0()
    {
      uint16_t r = 0;
      for (int i = 0; i < 10; i++)
        if (getBit(818 + i))
          r |= (1 << i);
      return r;
    }
    void setB0(uint16_t val)
    {
      uint16_t r = val & 0x3FF;
      for (int i = 0; i < 10; i++)
        setBitState(818 + i, r & (1 << i));
    }
    uint8_t getHEADER() { return getByte(810); }
#define HEADER_INVERTED
#ifndef HEADER_INVERTED
    void setHEADER(uint8_t val)
    {
      setByte(810, val, 8);
    }
#else
    void setHEADER(uint16_t val)
    {
      uint16_t r = val & 0xFF;
      for (int i = 0; i < 8; i++)
        setBitState(817 - i, r & (1 << i));
    }
#endif
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
    void setMASK(uint8_t level, uint64_t mask)
    {
      for (int i = 0; i < 64; i++)
      {
        setBitState(618 + i * 3 + level, ((mask >> i) & 1) != 0);
      }
    }
    bool getMASKChannel(uint8_t level, int ch)
    {
      return getBit(618 + (ch & 63) * 3 + level);
    }
    void setMASKChannel(uint8_t level, int ch, bool on)
    {
      int i = ch & 63;
      setBitState(618 + i * 3 + level, on);
    }

    uint8_t getSWSSC() { return getByte(581) & 0x7; }
    void setSWSSC(uint8_t val) { setByte(581, val & 0x7, 3); }

    uint8_t getPAGAIN(int ch) { return getByte(64 + 8 * ch); }
    void setPAGAIN(int ch, uint8_t val) { setByte(64 + 8 * ch, val, 8); }

    bool getCTEST(int b) { return getBit(b % 64); }
    void setCTEST(int b, bool on) { setBitState(b % 64, on); }

    bool getEN_OTAQ() { return getBit(612); }
    void setEN_OTAQ(bool on) { setBitState(612, on); }
    bool getDACSW() { return getBit(849); }
    void setDACSW(bool on) { setBitState(849, on); }
    bool getSEL0() { return getBit(603); }
    void setSEL0(bool on) { setBitState(603, on); }
    bool getCMDB2FSB2() { return getBit(588); }
    void setCMDB2FSB2(bool on) { setBitState(588, on); }
    bool getCMDB0FSB2() { return getBit(590); }
    void setCMDB0FSB2(bool on) { setBitState(590, on); }
    bool getENOCTRANSMITON1B() { return getBit(869); }
    void setENOCTRANSMITON1B(bool on) { setBitState(869, on); }
    bool getSEL1() { return getBit(604); }
    void setSEL1(bool on) { setBitState(604, on); }
    bool getCMDB2FSB1() { return getBit(596); }
    void setCMDB2FSB1(bool on) { setBitState(596, on); }
    bool getOTABGSW() { return getBit(850); }
    void setOTABGSW(bool on) { setBitState(850, on); }
    bool getSW50F0() { return getBit(611); }
    void setSW50F0(bool on) { setBitState(611, on); }
    bool getENOCDOUT2B() { return getBit(870); }
    void setENOCDOUT2B(bool on) { setBitState(870, on); }
    bool getSW50K0() { return getBit(608); }
    void setSW50K0(bool on) { setBitState(608, on); }
    bool getSMALLDAC() { return getBit(848); }
    void setSMALLDAC(bool on) { setBitState(848, on); }
    bool getSELENDREADOUT() { return getBit(865); }
    void setSELENDREADOUT(bool on) { setBitState(865, on); }
    bool getCMDB3FSB1() { return getBit(595); }
    void setCMDB3FSB1(bool on) { setBitState(595, on); }
    bool getPWRONBUFF() { return getBit(584); }
    void setPWRONBUFF(bool on) { setBitState(584, on); }
    bool getENOCDOUT1B() { return getBit(871); }
    void setENOCDOUT1B(bool on) { setBitState(871, on); }
    bool getSW50K2() { return getBit(591); }
    void setSW50K2(bool on) { setBitState(591, on); }
    bool getCMDB2SS() { return getBit(578); }
    void setCMDB2SS(bool on) { setBitState(578, on); }
    bool getTRIG1B() { return getBit(852); }
    void setTRIG1B(bool on) { setBitState(852, on); }
    bool getCMDB1SS() { return getBit(579); }
    void setCMDB1SS(bool on) { setBitState(579, on); }
    bool getPWRONPA() { return getBit(576); }
    void setPWRONPA(bool on) { setBitState(576, on); }
    bool getPWRONSS() { return getBit(585); }
    void setPWRONSS(bool on) { setBitState(585, on); }
    bool getRAZCHNINTVAL() { return getBit(857); }
    void setRAZCHNINTVAL(bool on) { setBitState(857, on); }
    bool getSW100K1() { return getBit(600); }
    void setSW100K1(bool on) { setBitState(600, on); }
    bool getCLKMUX() { return getBit(860); }
    void setCLKMUX(bool on) { setBitState(860, on); }
    bool getSW50F1() { return getBit(602); }
    void setSW50F1(bool on) { setBitState(602, on); }
    bool getPWRONFSB0() { return getBit(605); }
    void setPWRONFSB0(bool on) { setBitState(605, on); }
    bool getENOCTRANSMITON2B() { return getBit(868); }
    void setENOCTRANSMITON2B(bool on) { setBitState(868, on); }
    bool getENOCCHIPSATB() { return getBit(867); }
    void setENOCCHIPSATB(bool on) { setBitState(867, on); }
    bool getCMDB3SS() { return getBit(577); }
    void setCMDB3SS(bool on) { setBitState(577, on); }
    bool getDISCRI1() { return getBit(616); }
    void setDISCRI1(bool on) { setBitState(616, on); }
    bool getSW50F2() { return getBit(594); }
    void setSW50F2(bool on) { setBitState(594, on); }
    bool getSW100K0() { return getBit(609); }
    void setSW100K0(bool on) { setBitState(609, on); }
    bool getCMDB3FSB2() { return getBit(587); }
    void setCMDB3FSB2(bool on) { setBitState(587, on); }
    bool getSCON() { return getBit(859); }
    void setSCON(bool on) { setBitState(859, on); }
    bool getTRIG2B() { return getBit(851); }
    void setTRIG2B(bool on) { setBitState(851, on); }
    bool getSW100K2() { return getBit(592); }
    void setSW100K2(bool on) { setBitState(592, on); }
    bool getSW100F1() { return getBit(601); }
    void setSW100F1(bool on) { setBitState(601, on); }
    bool getTRIGEXTVAL() { return getBit(856); }
    void setTRIGEXTVAL(bool on) { setBitState(856, on); }
    bool getPWRONFSB2() { return getBit(606); }
    void setPWRONFSB2(bool on) { setBitState(606, on); }
    bool getRS_OR_DISCRI() { return getBit(617); }
    void setRS_OR_DISCRI(bool on) { setBitState(617, on); }
    bool getTRIG0B() { return getBit(853); }
    void setTRIG0B(bool on) { setBitState(853, on); }
    bool getPWRONFSB1() { return getBit(607); }
    void setPWRONFSB1(bool on) { setBitState(607, on); }
    bool getSW100F2() { return getBit(593); }
    void setSW100F2(bool on) { setBitState(593, on); }
    bool getPWRONW() { return getBit(586); }
    void setPWRONW(bool on) { setBitState(586, on); }
    bool getRAZCHNEXTVAL() { return getBit(858); }
    void setRAZCHNEXTVAL(bool on) { setBitState(858, on); }
    bool getDISCRI2() { return getBit(615); }
    void setDISCRI2(bool on) { setBitState(615, on); }
    bool getSELSTARTREADOUT() { return getBit(866); }
    void setSELSTARTREADOUT(bool on) { setBitState(866, on); }
    bool getSW50K1() { return getBit(599); }
    void setSW50K1(bool on) { setBitState(599, on); }
    bool getCMDB1FSB1() { return getBit(597); }
    void setCMDB1FSB1(bool on) { setBitState(597, on); }
    bool getENTRIGOUT() { return getBit(854); }
    void setENTRIGOUT(bool on) { setBitState(854, on); }
    bool getCMDB0SS() { return getBit(580); }
    void setCMDB0SS(bool on) { setBitState(580, on); }
    bool getSW100F0() { return getBit(610); }
    void setSW100F0(bool on) { setBitState(610, on); }
    bool getCMDB1FSB2() { return getBit(589); }
    void setCMDB1FSB2(bool on) { setBitState(589, on); }
    bool getOTAQ_PWRADC() { return getBit(613); }
    void setOTAQ_PWRADC(bool on) { setBitState(613, on); }
    bool getCMDB0FSB1() { return getBit(598); }
    void setCMDB0FSB1(bool on) { setBitState(598, on); }
    bool getDISCROROR() { return getBit(855); }
    void setDISCROROR(bool on) { setBitState(855, on); }
    bool getDISCRI0() { return getBit(614); }
    void setDISCRI0(bool on) { setBitState(614, on); }

    bool isEnabled()
    {
      if (_jasic.isMember("ENABLED"))
        return (_jasic["ENABLED"] == 1);
      else
        return true;
    }
    void setEnable(bool i) { _jasic["ENABLED"] = i; }
    void invertBits(hr2 *r)
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

    Json::Value &getJson() { return _jasic; }
    void setJson(Json::Value v)
    {
      _jasic = v;
      this->setFromJson();
    }

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
    int fd=::open(fname.c_str(),O_RDONLY);
      if (fd<0) 
        {
          fprintf(stderr,"%s  Cannot open file %s : return code %d \n",__PRETTY_FUNCTION__,fname.c_str(),fd);
        }
      int size_buf=::read(fd,_l,28*sizeof(uint32_t));
      ::close(fd);
  }
  private:
    uint32_t _l[28];
    uint32_t _li[28];
    Json::Value _jasic;
  };
};
