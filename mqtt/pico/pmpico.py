from umqtt.robust2 import MQTTClient
from usocket import socket
from machine import Pin,SPI,ADC,I2C
import network
import time
import oled
import bme280
import hih81310
import json
from writer import Writer
import freesans12  # Font to use
import ubinascii
#import settings
import genesysPico
import zupPico
import brooksPico
import cpwplus
#mqtt config

client_id = 'wiz'



class PmPico:
 
    def __init__(self,**kwargs):
        #
        self.topic_prefix= "pico_w5500/Test/"
        # parse JSON file
        self.settings=json.load(open("settings.json"))
        self.debug=False
        if "debug" in  self.settings.keys():
            self.debug=self.settings["debug"]==1
        if "id" in self.settings.keys():
            if "subid" in self.settings.keys():
                self.topic_prefix=self.settings["id"]+"/"+self.settings["subid"]+"/"
        #Oled
        self.oled_init()
        
        # network
        useWiznet=False
        if "wiznet" in self.settings.keys():
            s_dv=self.settings["wiznet"]
            if  s_dv["use"]==1:
                useWiznet=True
                self.w5x00_init()
                
        if not useWiznet:
            self.wifi_init()
        # MQTT
        if "mqtt" in self.settings.keys():
            self.mqtt_connect(self.settings["mqtt"]["server"])
        #display initialisation
        self.devices={}
        # brooks
        if "brooks" in self.settings.keys():
            s_dv=self.settings["brooks"]
            if  s_dv["use"]==1:
                self.brooks_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["baud"],s_dv["rst"],s_dv["devices"][0])
                self.devices["brooks"]={"period":s_dv["period"],"last":0,"measure":self.brooks_status,
                                         "callback":self.brooks.process_message}
                stv=self.brooks.view()
                self.publish("brooks/INFOS",stv,True)

        # cpwplus
        if "cpwplus" in self.settings.keys():
            s_dv=self.settings["cpwplus"]
            if  s_dv["use"]==1:
                self.devices["cpwplus"]={"period":s_dv["period"],"last":0,"measure":self.cpwplus_status}
                self.cpwplus_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["baud"])
                self.publish("cpwplus/INFOS",{"id":"adam_cpw+"},True)
        #Genesys
        if "genesys" in self.settings.keys():
            s_dv=self.settings["genesys"]
            if  s_dv["use"]==1:
                self.genesys_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["address"],s_dv["baud"])
                self.devices["genesys"]={"period":s_dv["period"],"last":0,"measure":self.genesys_status,
                                         "callback":self.genesys.process_message}
                stv=self.genesys.view()
                self.publish("genesys/INFOS",stv,True)

        #Zup
        if "zup" in self.settings.keys():
            s_dv=self.settings["zup"]
            if  s_dv["use"]==1:
                self.zup_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["address"],s_dv["baud"])
                self.devices["zup"]={"period":s_dv["period"],"last":0,"measure":self.zup_status,
                                         "callback":self.zup.process_message}
                stv=self.zup.view()
                self.publish("zup/INFOS",stv,True)
        #BME
        if "bme" in self.settings.keys():
            s_dv=self.settings["bme"]
            if  s_dv["use"]==1:               
                self.bme_init(s_dv["i2c"],s_dv["sda"],s_dv["scl"])
                self.devices["bme"]={"period":s_dv["period"],"last":0,"measure":self.bme_status,
                                     "callback":self.bme.process_message}
                stv=self.bme.view()
                self.publish("bme/INFOS",stv,True)
        #HIH
        if "hih" in self.settings.keys():
            s_dv=self.settings["hih"]
            if  s_dv["use"]==1:
                self.hih_init(s_dv["i2c"],s_dv["sda"],s_dv["scl"])
                self.devices["hih"]={"period":s_dv["period"],"last":0,"measure":self.hih_status,
                                     "callback":self.hih.process_message}                                

                stv=self.hih.view()
                self.publish("hih/INFOS",stv,True)

        #RP2040
        if "rp2040" in self.settings.keys():
            s_dv=self.settings["rp2040"]
            if  s_dv["use"]==1:
                self.devices["rp2040"]={"period":s_dv["period"],"last":0,"measure":self.rp2040_status}
                self.publish("rp2040/INFOS",{"id":"rp2040"},True)

        
    def oled_init(self):
        self.oledd = oled.OLED_1inch3()
        self.oledd.clear()
        time.sleep_ms(100)

        self.oledd.fill(0)
        self.writer = Writer(self.oledd, freesans12)  # verbose = False to suppress console output
        Writer.set_textpos(self.oledd, 0, 0)  # In case a previous test has altered this
        self.draw_string('PM Slow\nC.Combaret\nL.Mirabito\nInitialising')
        time.sleep(2)
        
    def draw_string(self,s_text):
        if (self.debug):
            print(s_text)
        self.oledd.clear()
        time.sleep_ms(100)

        self.oledd.fill(0)
        Writer.set_textpos(self.oledd, 0, 0)  # In case a previous test has altered this
        self.writer.printstring(s_text)
        self.oledd.show()
        time.sleep_ms(100)
    # brooks init
    def brooks_init(self,uartid,txp,rxp,baud,rst,d_id):
        # Uart nb / tx /rx
        self.brooks = brooksPico.brooksPico(uartid,txp,rxp,device_id=d_id,baud=baud,rst_pin=rst)
        st=self.brooks.status()
        sti=self.brooks.read_gas_type(1)
        print(sti)
        self.draw_string("Brooks\nFlow %.2f l/h" % (st["primary_variable"]))
        #print(st)      
        time.sleep(1)

    # cpwplus init
    def cpwplus_init(self,uartid,txp,rxp,baud):
        # Uart nb / tx /rx
        self.cpwplus = cpwplus.Cpwplus(uartid,txp,rxp,baud)
        st=self.cpwplus.process_message("STATUS")
        self.draw_string("CPWPLUS\nNet Weight %.2f kg" % (st["net"]))
        #print(st)      
        time.sleep(1)
    # genesys init
    def genesys_init(self,uartid,txp,rxp,address,baud):
        
        self.genesys = genesysPico.genesysPico(uartid,txp,rxp,address,baud)
        st=self.genesys.status()
        #print(st)
        self.draw_string("genesys\n%.1f %.1f %.1f %.1f\nStatus %s" %
                        (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        #print(st)
        time.sleep(1)
    # zup init port 2
    def zup_init(self,uartid,txp,rxp,address,baud):
        self.zup = zupPico.zupPico(uartid,txp,rxp,address,baud)
        st=self.zup.status()
        #print(st)
        self.draw_string("Zup\n%.1f %.1f %.1f %.1f\nStatus %s" %
                        (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        #print(st)
        time.sleep(1)
        
    # BME280 init
    def bme_init(self,i2cid,psda,pscl):
        self.i2c0=I2C(i2cid,sda=Pin(psda), scl=Pin(pscl), freq=100000)    #initializing the I2C method
        self.bme = bme280.BME280(i2c=self.i2c0)
        self.draw_string("BME280\ninitialised\nI2C0\nSDA4 SCL5" )
        time.sleep(2)
    # HIH81310 init
    def hih_init(self,i2cid,psda,pscl):
        pin_27 = Pin(pscl,pull=Pin.PULL_UP)
        pin_26 =Pin(psda,pull=Pin.PULL_UP)
        self.i2c1=I2C(1,sda=pin_26, scl=pin_27, freq=200000)    #initializing the I2C method
        pin_27 = Pin(pscl,pull=Pin.PULL_UP)
        pin_26 =Pin(psda,pull=Pin.PULL_UP)
        self.hih=hih81310.HIH81310(i2c=self.i2c1)
        self.draw_string("HIH81310\ninitialised\nI2C1\nSDA26 SCL27")
        time.sleep(2)
    
        
    def wifi_init(self):
 
        self.draw_string('SSID \n%s\n' % settings.ssid)
        wlan = network.WLAN(network.STA_IF)
        wlan.active(True)
        #DHCP
        lan_mac = wlan.config('mac')
        macaddress=ubinascii.hexlify(lan_mac).decode()
        #print(macaddress)
        
        self.draw_string("Macaddress WLAN\n%s" % str(macaddress))
        wlan.connect(settings.ssid,settings.password)
        while not wlan.isconnected():
            time.sleep(1)
        time.sleep(2)
        self.draw_string('joined to\n%s\n%s' % (settings.ssid,str(wlan.ifconfig()[0])))
        time.sleep(2)

    #W5x00 chip init
    def w5x00_init(self):
        spi=SPI(0,2_000_000, mosi=Pin(19),miso=Pin(16),sck=Pin(18))
        nic = network.WIZNET5K(spi,Pin(17),Pin(20)) #spi,cs,reset pin
        nic.active(True)
        #nic.connect()
        #None DHCP
        if self.settings["wiznet"]["dhcp"]==0:
            nic.ifconfig((self.settings["wiznet"]["ip"],'255.255.255.0',self.settings["wiznet"]["gw"],
                          '8.8.8.8'))
    
        #DHCP
        lan_mac = nic.config('mac')
        macaddress=ubinascii.hexlify(lan_mac).decode()
        if (self.debug):
            print(macaddress)
        
        self.draw_string("Wiznet IP\n%s" % str(nic.ifconfig()))
        #print('IP address :', nic.ifconfig())
        time.sleep(2)
        while not nic.isconnected():
            self.draw_string("Waiting\n%s" % str(nic.ifconfig()))
            #print("waiting for connection ...",macaddress)
            time.sleep(2)
            #print(nic.regs())
        self.draw_string("Connected\n%s" % str(nic.ifconfig()))
        time.sleep(5)

    # MQTT Call_back
    def mqtt_cb(self,topic, msg,retain,dup):
      print((topic, msg.decode("utf-8")))
      p_msg=json.loads(msg.decode("utf-8"))
      print(p_msg)
      if not "command" in p_msg.keys():
          return
      if not "device" in p_msg.keys():
          return
      self.draw_string("Command\ndev=%s\ncmd=%s" % (p_msg["device"],p_msg["command"]))
      #return
      # Process any command
      if p_msg["device"] in self.devices.keys():
          if "callback" in self.devices[p_msg["device"]].keys():
              self.devices[p_msg["device"]]["callback"](p_msg)
  
    # MQTT Connection
    def mqtt_connect(self,mqtt_server):
        self.draw_string('Connecting to\n%s \nMQTT Broker' % (mqtt_server))
        time.sleep(1)
        self.client = MQTTClient(client_id, mqtt_server, keepalive=6000)
        self.client.set_callback(self.mqtt_cb)

        while True:
            try:
                self.client.connect()
                self.draw_string('Connected to\n%s \nMQTT Broker'%(mqtt_server))
                time.sleep(2)
                break
            except OSError as e:
                self.draw_string("MQtt.\nFailed\nRetrying")
            time.sleep(5)
        topic=self.topic_prefix+"CMD"
        self.draw_string("Subscribing to \n %s "  % topic)
        self.client.subscribe(str.encode(topic))
        
        
    def check_connection(self,method):
        if self.client.is_conn_issue():
            print(method," MQTT issue... Reconnecting")
            while self.client.is_conn_issue():
                # If the connection is successful, the is_conn_issue
                # method will not return a connection error.
                self.client.reconnect()
            else:
                self.client.resubscribe()
  
    def publish(self,device_pub,msg,keep=False):
        topic_pub=self.topic_prefix+device_pub
        tmsg=json.dumps(msg)
        self.check_connection("Publish ")
        rc=self.client.publish(topic_pub.encode("utf8"), tmsg.encode("utf8"),retain=keep)

        
    def check_msg(self):
        self.check_connection("Check_Msg ")
        self.client.DEBUG=True
        try:
            self.client.check_msg()
        except:
            self.client.log()
            print("check msg error...reconnecting")
            
        
    def rp2040_status(self):
        conversion_factor = 3.3 / (65535)
        sensor_temp = ADC(4)
        reading = sensor_temp.read_u16() * conversion_factor 
        temperature = 27 - (reading - 0.706)/0.001721
        #res={}
        #res["T"]=temperature
        #topic_pub = 'pico_w5500/%s/rp2040' % self.settings["id"] 
        #tmsg=json.dumps(res)
        self.publish("rp2040",{"T":temperature})
        self.draw_string("RP2040\nProcessor\n%.1f C" % temperature)
        #print("RP2040\nProcessor\n%.1f C" % temperature)
        time.sleep(1)
    def bme_status(self):
        t, p, h = self.bme.read_hrvalues()
        #res={}
        #res["T"]=t
        #res["P"]=p
        #res["H"]=h
        #topic_pub ='pico_w5500/%s/bme280' % self.settings["id"]
        #tmsg=json.dumps(res)
        self.publish("bme", {"T":t,"P":p,"H":h})
        self.draw_string("BME280\nP=%.1f hPa\nT=%.1f C\nHum=%.1f %%" % (p,t,h))
        time.sleep(1)
    def hih_status(self):
        h,t=self.hih.read_sensor()
        #res={}
        #res["T"]=t
        #res["H"]=h
        #topic_pub = 'pico_w5500/%s/hih' % self.settings["id"]
        #tmsg=json.dumps(res)
        self.publish("hih", {"T":t,"H":h})
        self.draw_string("HIH81310\nT=%.1f C\nHum=%.1f %%" % (t,h))
        time.sleep(1)
    def genesys_status(self):
        st=self.genesys.status()
        
        #topic_pub = 'pico_w5500/%s/genesys' % self.settings["id"]
        #tmsg=json.dumps(st)
        self.publish("genesys", st)
        #self.client.publish(topic_pub.encode("utf-8"), tmsg.encode("utf8"))
        self.draw_string("genesys\n%.1f %.1f %.1f %.1f\nStatus %s" %
                         (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        time.sleep(1)
        #time.sleep(t_sleep)
    def zup_status(self):
        st=self.zup.status()
        
        #topic_pub = 'pico_w5500/%s/zup' % self.settings["id"]
        #tmsg=json.dumps(st)
        self.publish("zup", st)
        #self.client.publish(topic_pub.encode("utf-8"), tmsg.encode("utf8"))
        self.draw_string("zup\n%.1f %.1f %.1f %.1f\nStatus %s" %
                         (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        time.sleep(1)
        #time.sleep(t_sleep)
    def cpwplus_status(self):
        st=self.cpwplus.status()
        #topic_pub = 'pico_w5500/%s/cpwplus' % self.settings["id"]
        
        #tmsg=json.dumps(st)
        self.publish("cpwplus", st)
        #self.publish(topic_pub, tmsg)
        self.draw_string("CPWPLUS\nNet Weight %.2f kg" % (st["net"]))
        time.sleep(1)
    def brooks_status(self):
        bks_did=self.settings["brooks"]["devices"]
        if (len(bks_did)==1 and bks_did[0]==0):
            st=self.brooks.identity()
            #tmsg=json.dumps(st)
            self.publish("brooksid", st)
            
            self.draw_string("brooks disc\n Gas  %s \nID %d\n range %.2f" %
                         (st["gas_type"],st["device_id"],st["gas_flow_range"]))
            time.sleep(3)
            return
        # Normal readout
        for id in bks_did:
            self.brooks.use_device(id)
            sti=self.brooks.identity()
            st=self.brooks.status()
        
            #topic_pub = 'pico_w5500/%s/zup' % self.settings["id"]
            #tmsg=json.dumps(st)
            self.publish("brooks/%s" % sti["gas_type"], st)
            #self.client.publish(topic_pub.encode("utf-8"), tmsg.encode("utf8"))
            self.draw_string("brooks %s\n Set %.2f \nRead %.2f" %
                             (sti["gas_type"],st["setpoint_selected"],st["primary_variable"]))
            time.sleep(1)
        #time.sleep(t_sleep)
def main():
    pm=PmPico()
   
    it=0
    while True:
        for d in pm.devices.keys():
            pm.check_msg()
            tc=it*0.2
            di=pm.devices[d]
            if ((tc-di["last"])>di["period"]):
                di["last"]=tc
                if "measure" in di.keys():
                    di["measure"]()
                    print("Next iteration \n %d" % it)
        it=it+1
        #pm.client.check_msg()
        time.sleep_ms(200)
    pm.client.disconnect()

if __name__ == "__main__":
    main()


