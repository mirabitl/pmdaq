#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#undef USEWIRINGPI
#ifdef USEWIRINGPI
#include <wiringPi.h>
#include <wiringPiSPI.h>
#else
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#endif
#include <stdio.h>

/**
   x"00" => w_enableHigh0_7_reg 
   x"01" => w_enableHigh8_15_reg
   x"02" => w_enableHigh16_23_reg
   default = x"FFFFFF" -> tout actif
   x"03" => w_enableLow0_7_reg
   x"04" => w_enableLow8_15_reg
   x"05" => w_enableLow16_23_reg
   default =x"000000" -> tout inactif
   x"06" => w_TriggerSource_reg
   default = x"02"
   x"01" -> interne single
   x"02" -> interne multiple
   x"04" -> externe single 
   x"08" ->  externe multiple
   x"10" -> software 
   x"07" => w_SoftTrigger_reg
   default =x"00"
   x"01" -> bit de trigger
   x"08" => w_TriggerNumber0_7_reg
   x"09" => w_TriggerNumber8_15_reg
   x"0A" => w_TriggerNumber16_23_reg
   x"0B" => w_TriggerNumber24_31_reg
   default= x"000010"
   x"0C" => w_TriggerDelay0_7_reg
   x"0D" => w_TriggerDelay8_15_reg
   x"0E" => w_TriggerDelay16_23_reg
   x"0F" => w_TriggerDelay24_31_reg
   default =x"000000"
   x"10" => w_TriggerDuration0_7_reg
   x"11" => w_TriggerDuration8_15_reg
   x"12" => w_TriggerDuration16_23_reg
   x"13" => w_TriggerDuration24_31_reg
   default = x"0000FF"
   x"14" => w_mcp4021_value_reg
   default = x"00"
   bit 0-6 : value 
   bit 7 : 1: enable loading
   0 : do nothing
   x"15" => w_software_veto_reg
   default = x"00"
   bit 0: 0 : no pulse for external trigger  
   1 : external trigger enabled

*/

namespace febinj
{
#ifdef USEWIRINGPI
  class utilspi
  {
  public:
    static void write(struct wiringPiNodeStruct *node, int addr, int value)
    {

      unsigned char spiData[2];

      spiData[0] = (addr & 0x7f);
      spiData[1] = value & 0xff;

      wiringPiSPIDataRW(node->fd, spiData, 2);
    }

    static int read(struct wiringPiNodeStruct *node, int addr)
    {
      unsigned char spiData[2];

      spiData[0] = (addr & 0x7f) + 0x80;
      spiData[1] = 0;

      wiringPiSPIDataRW(node->fd, spiData, 2);
      return spiData[1];
    }
  };
  class board
  {
  public:
    board()
    {
      if (wiringPiSPISetup(0, 10000) < 0) // 10MHz Max
        return;

      node = wiringPiNewNode(64, 2);
      node->fd = 0;
      node->analogWrite = utilspi::write;
      node->analogRead = utilspi::read;
    }
    ~board()
    {
      ;
    }

    void setMask(uint32_t mask, uint32_t hr)
    {

      uint8_t c[3];
      for (int i = 0; i < 3; i++)
      {
        c[i] = (mask >> (i * 8)) & 0xFF;
        if (hr == 0)
        {
          utilspi::write(node, i, c[i]);
          fprintf(stderr, " HR = %d MASK= %x Byte %d ByteMask %x \n", hr, mask, i, c[i]);
        }
        else
        {
          utilspi::write(node, i + 3, c[i]);
          fprintf(stderr, " HR = %d MASK= %x Byte %d ByteMask %x \n", hr, mask, i + 3, c[i]);
        }
        usleep(1000);
      }
    }
    void setTriggerSource(uint32_t source)
    {
      utilspi::write(node, 6, source & 0xFF);
    }
    void softwareTrigger()
    {
      utilspi::write(node, 7, 1);
      usleep(100);
      utilspi::write(node, 7, 0);
    }
    void internalTrigger()
    {
      utilspi::write(node, 7, 2);
      usleep(100);
      utilspi::write(node, 7, 0);
    }
    void pauseExternalTrigger()
    {
      utilspi::write(node, 0x15, 0);
    }
    void resumeExternalTrigger()
    {
      utilspi::write(node, 0x15, 1);
    }

    void setNumberOfTrigger(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        utilspi::write(node, i + 0x8, c[i]);
        usleep(1);
      }
    }
    void setDelay(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        utilspi::write(node, i + 0xc, c[i]);
        usleep(1);
      }
    }
    void setDuration(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        utilspi::write(node, i + 0x10, c[i]);
        usleep(1);
      }
    }
    void setPulseHeight(uint32_t n)
    {

      uint8_t c = n & 0x7f + 128;
      utilspi::write(node, 0x14, c);
    }

  private:
    struct wiringPiNodeStruct *node;
  };
#else
  class board
  {
  public:
    board()
    {
      _bits = 8;
      _speed = 10000;
      int ret = 0;
      /*
	mode |= SPI_CPHA;
	mode |= SPI_CPOL;
	mode |= SPI_LSB_FIRST;
	mode |= SPI_CS_HIGH;
      */
      uint32_t mode = 0;

      _fd = open("/dev/spidev0.0", O_RDWR);
      if (_fd < 0)
      {
        perror("open fail");
        return;
      }

      ret = ioctl(_fd, SPI_IOC_WR_MODE32, &mode);
      if (ret == -1)
        perror("can't set spi mode");
      ret = ioctl(_fd, SPI_IOC_WR_BITS_PER_WORD, &_bits);
      if (ret == -1)
        perror("can't set bits per word");
      ret = ioctl(_fd, SPI_IOC_WR_MAX_SPEED_HZ, &_speed);
      if (ret < 0)
        perror("can't set max speed hz");
    }
    ~board()
    {
      close(_fd);
    }

    void spi_dev_Write(uint32_t addr, uint32_t value)
    {
      struct spi_ioc_transfer xfer[1];
      unsigned char buf[32];
      uint32_t status;

      memset(xfer, 0, sizeof xfer);
      memset(buf, 0, sizeof buf);

      buf[0] = (addr & 0x7f); // bit 7 = 0 for write
      buf[1] = value;

      xfer[0].speed_hz = _speed;
      xfer[0].bits_per_word = _bits;
      xfer[0].tx_buf = (unsigned long)buf;
      xfer[0].len = 2;

      /*	xfer[1].speed_hz=speed;
		xfer[1].bits_per_word=bits;
		xfer[1].rx_buf = (unsigned long) buf;
		xfer[1].len = 0;
      */

      status = ioctl(_fd, SPI_IOC_MESSAGE(1), &xfer);
      //	printf (" at 0x%x, set 0x%x\n",addr, value);
      //	getchar();
      if (status < 1)
        perror("can't send spi message");
    }

    uint32_t spi_dev_Read(uint32_t addr)
    {

      struct spi_ioc_transfer xfer[2];
      unsigned char buf[32];
      uint32_t status;

      memset(xfer, 0, sizeof xfer);
      memset(buf, 0, sizeof buf);

      buf[0] = (addr & 0x7f) + 0x80; // bit 7 = & for read

      xfer[0].speed_hz = _speed;
      xfer[0].bits_per_word = _bits;
      xfer[0].tx_buf = (unsigned long)buf;
      xfer[0].len = 1;

      xfer[1].speed_hz = _speed;
      xfer[1].bits_per_word = _bits;
      xfer[1].rx_buf = (unsigned long)buf;
      xfer[1].len = 1;

      status = ioctl(_fd, SPI_IOC_MESSAGE(2), &xfer);
      if (status < 1)
        perror("can't get  spi message");
      //	printf (" at 0x%x, get 0x%x\n",addr, buf[0]);
      //	getchar();

      return buf[0];
    }

    void setMask(uint32_t mask, uint32_t hr)
    {

      uint8_t c[3];
      for (int i = 0; i < 3; i++)
      {
        c[i] = (mask >> (i * 8)) & 0xFF;
        if (hr == 0)
        {
          spi_dev_Write(i, c[i]);
          fprintf(stderr, " HR = %d MASK= %x Byte %d ByteMask %x \n", hr, mask, i, c[i]);
        }
        else
        {
          spi_dev_Write(i + 3, c[i]);
          fprintf(stderr, " HR = %d MASK= %x Byte %d ByteMask %x \n", hr, mask, i + 3, c[i]);
        }
        usleep(1000);
      }
    }
    void setTriggerSource(uint32_t source)
    {
      spi_dev_Write(6, source & 0xFF);
    }
    void softwareTrigger()
    {
      spi_dev_Write(7, 1);
      usleep(100);
      spi_dev_Write(7, 0);
    }
    void internalTrigger()
    {
      spi_dev_Write(7, 2);
      usleep(100);
      spi_dev_Write(7, 0);
    }
    void pauseExternalTrigger()
    {
      spi_dev_Write(0x15, 0);
    }
    void resumeExternalTrigger()
    {
      spi_dev_Write(0x15, 1);
    }

    void setNumberOfTrigger(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        spi_dev_Write(i + 0x8, c[i]);
        usleep(1);
      }
    }
    void setDelay(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        spi_dev_Write(i + 0xc, c[i]);
        usleep(1);
      }
    }
    void setDuration(uint32_t n)
    {

      uint8_t c[4];
      for (int i = 0; i < 4; i++)
      {
        c[i] = (n >> (i * 8)) & 0xFF;
        spi_dev_Write(i + 0x10, c[i]);
        usleep(1);
      }
    }
    void setPulseHeight(uint32_t n)
    {

      uint8_t c = n & 0x7f + 128;
      spi_dev_Write(0x14, c);
    }

  private:
    uint32_t _fd;
    uint32_t _bits;
    uint32_t _speed;
  };
#endif
};
