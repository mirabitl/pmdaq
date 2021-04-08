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
extern "C" {

#include <linux/types.h>
  //#include <linux/i2c.h>
#include <linux/i2c-dev.h>
}

//#include <i2c/smbus.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define TEMP 0x00
#define CONFIG 0x01
#define CONFIG  0x01
#define DEV_ADDR 0x4A
#define HUM_ADDR 0x27

#define DELAY  5000000

// GPIO stuff

#define PINSELECT 4
#define LOW  0
#define HIGH 1
#define DIR_IN  0
#define DIR_OUT 1

using namespace std;

#include "stdafx.hh"
static LoggerPtr _logHih(Logger::getLogger("PMDAQ_HIH"));

class hih8000
{
public:
  hih8000()
  {
    Setup () ;
    _temperature[0]=0;
    _temperature[1]=0;
    _humidity[0]=0;
    _humidity[1]=0;
  }
 
  ~hih8000(){
    GPIOUnexport(PINSELECT);
  }
  int GPIOExport(int pin)
  {
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;
    
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open export for writing!\n");
      return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
  }
  int  GPIOUnexport(int pin)
  {
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open unexport for writing!\n");
      return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
  }

  int GPIODirection(int pin, int dir)
  {
    static const char s_directions_str[]  = "in\0out";
#define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio direction for writing!\n");
      return(-1);
    }

    if (-1 == write(fd, &s_directions_str[DIR_IN == dir ? 0 : 3], DIR_IN == dir ? 2 : 3)) {
      fprintf(stderr, "Failed to set direction!\n");
      return(-1);
    }

    close(fd);
    return(0);
  }
  int GPIORead(int pin)
  {
#define VALUE_MAX 30
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
      return(-1);
    }

    if (-1 == read(fd, value_str, 3)) {
      fprintf(stderr, "Failed to read value!\n");
      return(-1);
    }

    close(fd);

    return(atoi(value_str));
  }

  int GPIOWrite(int pin, int value)
  {
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio value for writing!\n");
      return(-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
      fprintf(stderr, "Failed to write value!\n");
      return(-1);
    }

    close(fd);
    return(0);
  }

  int Setup ()
  {
    int temp1, temp2;

    if (wiringPiSetup ()<<0) 
      {
	LOG4CXX_INFO(_logHih," unable to open  GPIO");
	return -1 ;
      }

    std::string fi2c="/dev/i2c-1";
    if ((_fd=open(fi2c.c_str(),O_RDWR))<0)
      {
	LOG4CXX_INFO(_logHih," unable to open I2C device "<<fi2c<< " _fd="<<_fd );
	return -1 ;
      }


    // First select the chip
    
    GPIOExport(PINSELECT);
    GPIODirection(PINSELECT,DIR_OUT);
    GPIOWrite (PINSELECT, 1) ; 

    usleep(DELAY); // 2 secs before calling i2c setup required
    
    int addr = HUM_ADDR;          //<<<<<The I2C address of the slave
    if (ioctl(_fd, I2C_SLAVE, addr) < 0)
        {
	LOG4CXX_INFO(_logHih," unable to open I2C at "<<HUM_ADDR<< " _fd="<<_fd );//DEV_ADDR);
	return -1 ;
      } 
    else 
      printf ("fd=%d\n",_fd);

    return 0 ;
  }

  int Read()
  {
    unsigned char buf[4];                 /* Buffer for data read/written on the i2c bus */

    GPIOWrite (PINSELECT, 1) ;  // capteur sur la partir etroite de la carte
    //  if ((i2c_smbus_write_quick(_fd, 0)) != 0)
    // {
    if (wiringPiI2CWrite (_fd, 0)!=0)
      {
	printf("Error writing bit to i2c slave 1 \n");
	return -1;
	// exit(1);
      }
    usleep(100000);
    if (read(_fd, buf, 4) < 0)
      {
	printf("Unable to read from slave\n");
	//    exit(1);
	return -1;
      }
    else
      {
	int reading_hum = (buf[0] << 8) + buf[1];
	double humidity = reading_hum / 16382.0 * 100.0;
	//printf("Humidity: %f\n", humidity);

	int reading_temp = (buf[2] << 6) + (buf[3] >> 2);
	double temperature = reading_temp / 16382.0 * 165.0 - 40;
	printf("Temperature 1: %f %f\n", temperature,humidity);
	_humidity[1] =  (humidity);
	_temperature[1] = (temperature) + 273.15;
        //return 0;
      }




    


    GPIOWrite (PINSELECT, 0) ;  // capteur sur la partir large de la carte
    //if ((i2c_smbus_write_quick(_fd, 0)) != 0)
    //{
    if (wiringPiI2CWrite (_fd, 0)!=0)
      {   
	printf("Error writing bit to i2c slave\n");
	//exit(1);
      }
    usleep(100000);
    if (read(_fd, buf, 4) < 0)
      {
	printf("Unable to read from slave\n");
	//    exit(1);
      }
    else
      {
	int reading_hum = (buf[0] << 8) + buf[1];
	double humidity = reading_hum / 16382.0 * 100.0;
	//printf("Humidity: %f\n", humidity);
	
	int reading_temp = (buf[2] << 6) + (buf[3] >> 2);
	double temperature = reading_temp / 16382.0 * 165.0 - 40;
	//printf("Temperature: %f\n", temperature);
	printf("Temperature 0: %f %f\n", temperature,humidity);
	_humidity[0] =  humidity;
	_temperature[0] =  temperature + 273.15;
      }

    return 0;
  }

 
  double humidity(int i){return _humidity[i];}
  double temperature(int i){return _temperature[i];}
private:
  int _fd;
  double _humidity[2];
  double _temperature[2];

};
