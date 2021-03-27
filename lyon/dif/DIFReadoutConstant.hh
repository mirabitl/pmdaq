#ifndef _DIFReadoutConstant_h
#define _DIFReadoutConstant_h
#include <stdint.h>
#define DIF_FIRMWARE_VERSION 12

#if DIF_FIRMWARE_VERSION <= 12 

#define DIF_ID_SHIFT     1
#define DIF_DTC_SHIFT    2
#define DIF_GTC_SHIFT    10
#define DIF_BCID_SHIFT   14
#define DIF_GLOBAL_HEADER_SHIFT 0
#define DIF_DIFTEMP_SHIFT  33 
#endif



#define HARDROCV1_SLC_FRAME_SIZE 				72				// size (in byte) of a slow control frame. 72 is for a hardroc v1
#define HARDROCV2_SLC_FRAME_SIZE 				109				// size (in byte) of a slow control frame. 109 is for a hardroc v2
#define MICROROC_SLC_FRAME_SIZE 				74				// size (in byte) of a slow control frame. 109 is for a hardroc v2

#define DIRAC_SLC_FRAME_SIZE 					23				// size (in byte) of a slow control frame. 23 is for a dirac V2
#define MAX_NB_OF_ASICS 							48				// max number of hardrocs per dif that the system can handle
#define MAX_NB_OF_DIFS 								200				// max number of difs that the system can handle


#define MAX_ANALOG_DATA_SIZE 					1024*64*2+20//MAX_NB_OF_ASICS*64*2+20
#define MAX_FW_HEADER_SIZE 						50
#define ASIC_MEM_DEPTH								128				// memory depth of one asic . 128 is for hardroc v1

#define MAX_EVENT_SIZE 						(HARDROCV2_SLC_FRAME_SIZE+1)*MAX_NB_OF_ASICS+(20*ASIC_MEM_DEPTH+2)*MAX_NB_OF_ASICS+3+MAX_FW_HEADER_SIZE+2+MAX_ANALOG_DATA_SIZE+50

#define SOAP_DHCAL_DATA "DHCAL_DATA"

// System Definition
#define SYST_DHCAL                 	 	0x01

// Subsystem definition
#define SUB_SYST_HARDROC           	 	0x01		
#define SUB_SYST_DIF 									0x01

#define VERS_DIF                   	 	0x01

#define DATA_FORMAT_VERSION         	10 			//  version of current dataformat both for readout and backup file
// 1 for initial version
// 2 for version with global trigger counter and dif trigger counter
// 3 for version with timedif				
// 4 is for version with counters
// 5 is for version with A3 or C3 when uncomplete frame
// 6 is for version with SLC in header of data
// 7 with absolute BCID
// 8 with DIF ID after 0xb1 and only one timestamp after version byte
// 9 : with data format on 4 bytes and millisecond timstamp
// 10 : with DIF fw ersion
#define I2O_HARDROC_DIGITAL_DATA 	 	 	0xF0
#define I2O_HARDROC_ANALOG_DATA 	 		0xF1

#define HARDROC_DIGITAL_DATA_EMIT  		0xCA
#define HARDROC_DIGITAL_DATA_REPLY 		0xAC

/*
const U16 I2O_DHCAL_DATA =   0xF0;
const U16 DHCAL_I2O_DATA_EMIT  =  0XCA;
const U16 DHCAL_I2O_DATA_REPLY = 0XAC;
*/
#define I2O_DHCAL_DATA    						0xF0

#define DHCAL_I2O_DATA_EMIT   				0XCA
#define DHCAL_I2O_DATA_REPLY  				0XAC
#define DHCAL_I2O_DATA_NO_REPLY 			0xAD

//#define HARDROC_DIGITAL_DATA			  0x20
//#define HARDROC_ANALOG_DATA			  	0x30
#define DIF_DATA_CMD									0x40



//#define ASIC_TYPE											2					// 1 is for Hardroc V1
// 2 is for Hardroc V2 abd V2b
// 10 is for DiracV2


#define NB_OF_ASICS_PER_SLAB				48				// number of hardrocs per slab . 24 is for lyon version
#define NB_Of_CHANNELS_PER_ASIC			64				// channels per asics

#define NB_OF_XPADS_PER_ASIC				8         // number of pads per rows for 1 hardroc
#define NB_OF_YPADS_PER_ASIC				8         // number of pads per column for 1 hardroc

#define MAX_NB_OF_ASIC_PER_ROW		  8					// max  allowed number of hardroc for 1 row
#define MAX_NB_OF_ASIC_PER_COL		  8					// max  allowed number of hardroc for 1 column

#define DEBUG_LCIO															// if variable defined, backup file will be generated
#define BACKUP_FILE

//#define DUMMY_DIF																// if dummy DIF is used instead of real one
#undef DUMMY_DIF

// monitoring parameters
#define ShuntIDif											0.05
#define ShuntISlab										0.02


#define REINIT_RECV_TIMEOUT_MS 				1000

typedef unsigned char SingleHardrocV2ConfigurationFrame[HARDROCV2_SLC_FRAME_SIZE];		

typedef struct {
  uint32_t id;
  uint32_t nbasic;
  SingleHardrocV2ConfigurationFrame slow[48];

} DIFDbInfo;




#define SHM_NDIF_MAX 127
#define SHM_FREE 0
#define SHM_LOCK 1
#define SHM_READY 2
#define SHM_READING 3

#define SHM_FLAG 0 
#define SHM_READ_NUMBER 1
#define SHM_EVENT_NUMBER 2
#define SHM_BX_NUMBER 3
#define SHM_APV_ADDRESS 4
#define SHM_BUFFER_SIZE 5
#define SHM_EVENT_ADDRESS 6

#define SHM_EVENT_SIZE    0x20000
#define SHM_EVENT_SHIFT   SHM_EVENT_SIZE/4
#define SHM_MAX_SLOT  50
#define SHM_EXTRA_SLOT  5
#define SHM_SLOT_SPY  (SHM_MAX_SLOT+0)
#define SHM_NB_DEBUG  10
#define SHM_SLOT_DEBUG  (SHM_MAX_SLOT+1)
#define SHM_DIF_SHIFT   (SHM_EVENT_SHIFT*(SHM_MAX_SLOT+SHM_EXTRA_SLOT))




typedef struct
{
  uint32_t id;
  uint32_t status;
  uint32_t slc;
  uint32_t gtc;
  uint64_t bcid;
  uint64_t bytes;
  char host[80];
} DIFStatus;

#endif

