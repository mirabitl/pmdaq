 
import time
from ustruct import unpack, unpack_from
from array import array
 
# HIH81310 default address.
HIH_I2CADDR = 0x27
 
 
class HIH81310:
 
    def __init__(self,
                 address=HIH_I2CADDR,
                 i2c=None,
                 **kwargs):
        self.address = address
        if i2c is None:
            raise ValueError('An I2C object is required.')
        self.i2c = i2c
 
        # temporary data holders which stay allocated
        self._l1_barray = bytearray(1)
        self._l4_barray = bytearray(4)
        # callback
        self.cb={}
        self.cb["STATUS"]=self.status
        self.cb["VIEW"]=self.view
        
    def status(self):
        h,t=self.read_sensor()
        return {"T":t,"H":h}
    def view(self):
        return {"id":"hih81310","cmds":self.cb.keys()}
    def process_message(self,msg):
        rep={}
        cmd=msg["command"]
        params=None
        if "params" in msg:
            params=msg["params"]
        if not cmd in self.cb.keys():
            print("unknown command",cmd)
            return rep
        if params == None:
            return self.cb[cmd]()
        else:
            return self.cb[cmd](params)
        return rep
 
    def read_sensor(self):
        """ Reads the data from the sensor.
 
           
            Returns:
                result[2]=[humidity,temperature]
        """
 
        self._l1_barray[0] = 0
        self.i2c.writeto(self.address,self._l1_barray,False)
        time.sleep_ms(5000)
        self.i2c.readfrom_into(self.address,self._l4_barray,False)
        print(self._l4_barray)
        buf=[]
        buf.append(self._l4_barray[0])
        buf.append(self._l4_barray[1])
        buf.append(self._l4_barray[2])
        buf.append(self._l4_barray[3])

        
        tstatus = buf[0]>>6;
        print("tstatus =  %.2x %.2x %.2x %.2x " % (buf[0],buf[1],buf[2],buf[3]))
        humidity = -100
        temperature = -100
        if (tstatus ==0): 
            thum_raw= (( buf[0]&0x3F)<<8)+buf[1]
            thum_raw=thum_raw&0x3FFF
            ttemp_raw=(buf[2]<<6)+(buf[3]>>2)
            ttemp_raw=ttemp_raw&0x3FFF
            humidity = thum_raw/163.82
            temperature = ttemp_raw*165./16382.-40
            print("%d %d %f %f\n" %(thum_raw, ttemp_raw, humidity, temperature)) 


        result=[]
        result.append(humidity)
        result.append(temperature)
        return result
 
   
