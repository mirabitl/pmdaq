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
extern "C"
{

#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>

}
#include <sys/ioctl.h>


//#include <i2c/smbus.h>
#include <gpiod.h>

#define DEV_ADDR 0x4A
#define HUM_ADDR 0x27

#define DELAY 2000000

// GPIO stuff

#define PINSELECT 23
#define ExitOnFail 1
#define DisplayFailMessage 1
using namespace std;

class hihdriver
{
public:
  hihdriver()
  {
    _fd = -2;
    Setup();
    _temperature[0] = 0;
    _temperature[1] = 0;
    _humidity[0] = 0;
    _humidity[1] = 0;
  }

  ~hihdriver()
  {
    gpiod_line_release(_lineSel);
    if (_fd > -1)
      close(_fd);
  }
  int Setup()
  {
    int temp1, temp2;

    const char *chipname = "gpiochip0";

    _chip = gpiod_chip_open_by_name(chipname);
    _lineSel = gpiod_chip_get_line(_chip, PINSELECT);
    gpiod_line_request_output(_lineSel, "example1", 0);

    std::string fi2c = "/dev/i2c-1";
    if ((_fd = open(fi2c.c_str(), O_RDWR)) < 0)
      {
	std::cerr<< " unable to open I2C device " << fi2c << " _fd=" << _fd<<std::endl;
	return -1;
      }

    // First select the chip

    gpiod_line_set_value(_lineSel, 1);

    usleep(DELAY); // 2 secs before calling i2c setup required

    int addr = HUM_ADDR; //<<<<<The I2C address of the slave
    if (ioctl(_fd, I2C_SLAVE, addr) < 0)
      {
	std::cerr<<" unable to open I2C at " << HUM_ADDR << " _fd=" << _fd<<std::endl; 
	return -1;
      }
    else
      printf("fd=%d\n", _fd);

    return 0;
  }
  int chip_read(uint32_t chip)
  {
    
    unsigned char buf[20]; /* Buffer for data read/written on the i2c bus */
    gpiod_line_set_value(_lineSel, chip);
    ::usleep(1000000);
    int val=gpiod_line_get_value(_lineSel);
    printf(" Chip %d GPIO line value %d \n",chip,val);
    //getchar();
    buf[0]=HUM_ADDR;
    //buf[0]=0;
    //write(_fd,buf,1);
     if ((i2c_smbus_write_quick(_fd, 0)) != 0)
       {
     	printf("Error writing bit to i2c slave\n");
     	//exit(1);
       }
     ::usleep(400000);
    int nr=read(_fd, buf, 4);
    if (nr < 0)
      {
	fprintf(stderr, "Unable to read from slave %d\n",nr);
	//    exit(1);
	return -1;
      }
    else
      {
	printf("Bytes read %d \n",nr);
	for (int k=0;k<nr;k++)
	  printf("Buffer %d : %2x \n",k,buf[k]);
	int reading_hum = (buf[0] << 8) + buf[1];
	reading_hum=reading_hum&0x3FFF;
	double humidity = reading_hum / 16382.0 * 100.0;
	printf("Humidity %d: %f\n",chip, humidity);

	int reading_temp = (buf[2] << 6) + (buf[3] >> 2);
	double temperature = reading_temp / 16382.0 * 165.0 - 40;
	printf("Temperature %d: %f %f\n",chip, temperature, humidity);
	_humidity[chip] = (humidity);
	_temperature[chip] = (temperature) + 273.15;
	return 0;
      }
  }
  int Read()
  {
    int rc = chip_read(1);
    //if (rc != 0)
    //  return rc;
    rc = chip_read(0);
    return rc;
  }

  double humidity(int i) { return _humidity[i]; }
  double temperature(int i) { return _temperature[i]; }
private:
  int _fd;
  double _humidity[2];
  double _temperature[2];
  struct gpiod_chip *_chip;
  struct gpiod_line *_lineSel;
  int _handle;
};

