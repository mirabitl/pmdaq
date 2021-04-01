import socket
HEADER=0;LEN=1;TRANS=3;CMD=4;PAYLOAD=6;
WRITEREG=1;READREG=2;SLC=4;DATA=8;ACKNOWLEDGE=16;ERROR=32;
TEST=0x0;ID=0x1;
MASK=0x2;
SPILL_CNT=0x3;
ACQ_CTRL=0x4;
SPILL_ON=0x5;
SPILL_OFF=0x6;
CHANNEL_ENABLE=0x7;
CALIB_CTRL=0x8;
CALIB_NWIN=0xA;
RESET_FE=0xC;
WIN_CTRL=0xD;
TRG_EXT_DELAY=0xE;
TRG_EXT_LEN=0xF;
BUSY_0=0x10;
EN_BUSY_TRG=0x20;
DEBOUNCE_BUSY=0x21;
TDC_CTRL=0x30;
TDC_COARSE=0x31;
TDC_T1=0x32;TDC_CNT1=0x33;
TDC_T2=0x34;TDC_CNT2=0x35;
TDC_T3=0x36;TDC_CNT3=0x37;
TDC_T4=0x38;TDC_CNT4=0x39;
TDC_T5=0x3A;TDC_CNT5=0x3B;
TDC_T6=0x3C;TDC_CNT6=0x3D;
TDC_CAL1=0x40;TDC_CAL2=0x41;
VERSION=0x100;
RESET_FSM=0x60

class mbmdcc:
    def connect(self,ipad,port=10002):
        self.sock = socket.socket() 
        self.sock.connect((ipad, port))
        return 0
    def disconnect(self):
        self.sock.close()
        return 0
    def build_message(self,command,payload):
        buf=bytearray()
        blen=len(payload)+PAYLOAD+1
        buf.append(0x28)
        buf.extend(blen.to_bytes(length=2,byteorder='big'))
        buf.append(0)
        buf.append(command)
        buf.append(0)
        buf[PAYLOAD:PAYLOAD]=payload
        buf.append(0)
        buf.append(0)
        buf.append(0)
        buf.append(0x29)
        blen=len(buf)
        buf[1]=(blen>>8)&0xFF
        buf[2]=blen&0xFF
        #print(" Message ", buf, len(buf))
        return buf
    
    def send_message(self,buf):
        self.sock.sendall(buf)
        return 0
    def read_message(self):
        count=32
        self.sock.settimeout(5)
        newbuf = self.sock.recv(32)
        #print(newbuf)
        return newbuf
    
    def read_register(self,address):
        payload=bytearray()

        payload.extend(address.to_bytes(length=2,byteorder='big'))
        dummy=0
        payload.extend(dummy.to_bytes(length=4,byteorder='big'))
        #print("PAYLOAD",payload)
        b=self.build_message(READREG,payload)
        #print(b)
        rep=self.send_message(b)
        brep=self.read_message()
        #print(brep[PAYLOAD+2:PAYLOAD+6])
        byte_val=brep[PAYLOAD+2:PAYLOAD+6]
        irep=int.from_bytes(byte_val, "big")
        return irep
    def write_register(self,address,value):
        payload=bytearray();
        payload.extend(address.to_bytes(length=2,byteorder='big'))
        payload.extend(value.to_bytes(length=4,byteorder='big'))
        b=self.build_message(WRITEREG,payload)
        rep=self.send_message(b)
        brep=self.read_message()
        print(brep)
        byte_val=brep[PAYLOAD+2:PAYLOAD+6]
        irep=int.from_bytes(byte_val, "big")

        return irep
    def version(self):
        return self.read_register(VERSION);
    def id(self):
        return self.read_register(ID);
    def mask(self):
        return self.read_register(MASK);
    def maskTrigger(self):
        self.write_register(MASK,0xFFFFFFFF);
    def unmaskTrigger(self):
        self.write_register(MASK,0x0);
    def spillCount(self):
        return self.read_register(SPILL_CNT);
    def resetCounter(self):
        self.write_register(ACQ_CTRL,0x1);
        self.write_register(ACQ_CTRL,0x0);
    def spillOn(self):
        return self.read_register(SPILL_ON);
    def spillOff(self):
        return self.read_register(SPILL_OFF);
    def setSpillOn(self,nc):
        self.write_register(SPILL_ON,nc);
    def setSpillOff(self,nc):
        self.write_register(SPILL_OFF,nc);
    def Channels(self):
        return self.read_register(CHANNEL_ENABLE);
    def setChannels(self,nc):
        self.write_register(CHANNEL_ENABLE,nc);
    def calibOn(self):
        self.write_register(CALIB_CTRL,0x2);
    def calibOff(self):
        self.write_register(CALIB_CTRL,0x0);
    def calibCount(self):
        return self.read_register(CALIB_NWIN);
    def setCalibCount(self,nc):
        self.write_register(CALIB_NWIN,nc);

    def setCalibRegister(self,nc):
        self.write_register(CALIB_CTRL,nc);

    def hardReset(self):
        return self.read_register(RESET_FE);
    def setHardReset(self,nc):
        self.write_register(RESET_FE,nc);

    def setSpillRegister(self,nc):
        self.write_register(WIN_CTRL,nc);
    def spillRegister(self):
        return self.read_register(WIN_CTRL);

    def setTriggerDelay(self):
        self.write_register(TRG_EXT_DELAY,nc);
        
    def triggerDelay(self):
        return self.read_register(TRG_EXT_DELAY);

    def setTriggerBusy(self,nc):
        self.write_register(TRG_EXT_LEN,nc);

    def triggerBusy(self):
        return self.read_register(TRG_EXT_LEN);

    def setExternalTrigger(self,nc):
        self.write_register(EN_BUSY_TRG,nc);

    def externalTrigger(self):
        return self.read_register(EN_BUSY_TRG);

    def resetFSM(self,b):
        self.write_register(RESET_FSM,b);
    def resetTDC(self,b):
        self.write_register(RESET_FE,b);
    def busyCount(self,b):
        return self.read_register(BUSY_0+(b&0xF));

    def readStatus(self):
        self.status={}
        self.status["version"]=self.version()
        self.status["id"]=self.id()
        self.status["mask"]=self.mask()
        self.status["hard"]=self.hardReset()
        self.status["spill"]=self.spillCount()
        self.status["busy1"]=self.busyCount(1)
        self.status["busy2"]=self.busyCount(2)
        self.status["busy3"]=self.busyCount(3)
        self.status["busy4"]=self.busyCount(4)
        self.status["busy5"]=self.busyCount(5)
        self.status["busy6"]=self.busyCount(6)
        self.status["busy7"]=self.busyCount(7)
        self.status["busy8"]=self.busyCount(8)
        self.status["busy9"]=self.busyCount(9)
        self.status["busy10"]=self.busyCount(10)
        self.status["spillon"]=self.spillOn()
        self.status["spilloff"]=self.spillOff()
        self.status["channels"]=self.Channels()
        self.status["calib"]=self.calibCount()
        self.status["spillreg"]=self.spillRegister()
        self.status["trigdelay"]=self.triggerDelay()
        self.status["external"]=self.externalTrigger()

    def dump(self):
        self.readStatus()
        for k,v in self.status.items():
            print("\t ",k,v)


    def defaults(self):
        self.setChannels(3)
        self.setSpillOn(2000000)
        self.setSpillOff(20000)
        self.setSpillRegister(64)
        self.resetCounter()
        self.maskTrigger()
