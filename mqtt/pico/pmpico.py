from umqtt.simple import MQTTClient
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
import genesys
import zup
import cpwplus
#mqtt config

client_id = 'wiz'



class PmPico:
 
    def __init__(self,**kwargs):
        
        #Oled
        self.oled_init()
        # parse JSON file
        self.settings=json.load(open("settings.json"))
        self.devices=[]
        # cpwplus
        if "cpwplus" in self.settings.keys():
            s_dv=self.settings["cpwplus"]
            if  s_dv["use"]==1:
                self.devices.append("cpwplus")
                self.cpwplus_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["baud"])
        #Genesys
        if "genesys" in self.settings.keys():
            s_dv=self.settings["genesys"]
            if  s_dv["use"]==1:
                self.devices.append("genesys")
                self.genesys_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["address"],s_dv["baud"])

        #Zup
        if "zup" in self.settings.keys():
            s_dv=self.settings["zup"]
            if  s_dv["use"]==1:
                self.devices.append("zup")
                self.zup_init(s_dv["uart"],s_dv["tx"],s_dv["rx"],s_dv["address"],s_dv["baud"])
        #BME
        if "bme" in self.settings.keys():
            s_dv=self.settings["bme"]
            if  s_dv["use"]==1:
                self.devices.append("bme")
                self.bme_init(s_dv["i2c"],s_dv["sda"],s_dv["scl"])
        #HIH
        if "hih" in self.settings.keys():
            s_dv=self.settings["hih"]
            if  s_dv["use"]==1:
                self.devices.append("hih")
                self.bme_init(s_dv["i2c"],s_dv["sda"],s_dv["scl"])
        #HIH
        if settings.useHIH:
            self.hih_init()
        # network
        if settings.useWiznet:
            self.w5x00_init()
        else:
            self.wifi_init()
        # Time initialisation
        self.last_cpwplus=0
        self.last_genesys=0
        self.last_zup=0
        self.last_hih=0
        self.last_bme=0
        self.last_temp=0
    #display initialisation
    def oled_init(self):
        self.oledd = oled.OLED_1inch3()
        self.oledd.clear()
        time.sleep_ms(100)

        self.oledd.fill(0)
        self.writer = Writer(self.oledd, freesans12)  # verbose = False to suppress console output
        Writer.set_textpos(self.oledd, 0, 0)  # In case a previous test has altered this
        self.draw_string('PM Slow\nC.Combaret\nL.Mirabito\nInitialising')
        time.sleep(2)
    # cpwplus init
    def cpwplus_init(self,uartid,txp,rxp,baud):
        # Uart nb / tx /rx
        self.cpwplus = cpwplus.Cpwplus(uartid,txp,rxp,baud)
        st=self.cpwplus.process_message("STATUS")
        self.draw_string("CPWPLUS\nNet Weight %.2f kg" % (st["net"]))
        print(st)      
        time.sleep(1)
    # genesys init
    def genesys_init(self,uartid,txp,rxp,address,baud):
        
        self.genesys = genesysPico.genesysPico(uartid,txp,rxp,address,baud)
        st=self.genesys.process_message("STATUS")
        #print(st)
        self.draw_string("genesys\n%.1f %.1f %.1f %.1f\nStatus %s" %
                        (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        print(st)
        time.sleep(1)
    # zup init port 2
    def zup_init(self,uartid,txp,rxp,address,baud):
        self.zup = zupPico.zupPico(uartid,txp,rxp,address,baud)
        st=self.zup.process_message("STATUS")
        #print(st)
        self.draw_string("Zup\n%.1f %.1f %.1f %.1f\nStatus %s" %
                        (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        print(st)
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
    
    def draw_string(self,s_text):
        self.oledd.clear()
        time.sleep_ms(100)

        self.oledd.fill(0)
        Writer.set_textpos(self.oledd, 0, 0)  # In case a previous test has altered this
        self.writer.printstring(s_text)
        self.oledd.show()
        time.sleep_ms(100)
        
    def wifi_init(self):
 
        self.draw_string('SSID \n%s\n' % settings.ssid)
        wlan = network.WLAN(network.STA_IF)
        wlan.active(True)
        #DHCP
        lan_mac = wlan.config('mac')
        macaddress=ubinascii.hexlify(lan_mac).decode()
        print(macaddress)
        
        self.draw_string("Macaddress WLAN\n%s" % str(macaddress))
        wlan.connect(settings.ssid,settings.password)
        while not wlan.isconnected():
            time.sleep(1)
        time.sleep(2)
        self.draw_string('joined to\n%s\n%s' % (settings.ssid,str(wlan.ifconfig()[0])))
        time.sleep(5)

    #W5x00 chip init
    def w5x00_init(self):
        spi=SPI(0,2_000_000, mosi=Pin(19),miso=Pin(16),sck=Pin(18))
        nic = network.WIZNET5K(spi,Pin(17),Pin(20)) #spi,cs,reset pin
        nic.active(True)
        #nic.connect()
        #None DHCP
        #nic.ifconfig(('192.168.11.2','255.255.255.0','192.168.11.1','8.8.8.8'))
    
        #DHCP
        lan_mac = nic.config('mac')
        macaddress=ubinascii.hexlify(lan_mac).decode()
        print(macaddress)
        
        self.draw_string("Wiznet IP\n%s" % str(nic.ifconfig()))
        print('IP address :', nic.ifconfig())
        time.sleep(2)
        while not nic.isconnected():
            self.draw_string("Waiting\n%s" % str(nic.ifconfig()))
            print("waiting for connection ...",macaddress)
            time.sleep(2)
            #print(nic.regs())
        self.draw_string("Connected\n%s" % str(nic.ifconfig()))
        time.sleep(5)

    # MQTT Call_back
    def mqtt_cb(self,topic, msg):
      print((topic, msg.decode("utf-8")))
      p_msg=json.loads(msg.decode("utf-8"))
      print(p_msg)
      if not "command" in p_msg.keys():
          return
      if not "device" in p_msg.keys():
          return
      self.draw_string("Command\ndev=%s\ncmd=%s" % (p_msg["device"],p_msg["command"]))
      # Process any command
      if (p_msg["device"]=="genesys" and settings.useGenesys):
          st=self.genesys.process_message(p_msg)
      if (p_msg["device"]=="zup" and settings.useZup):
          st=self.zup.process_message(p_msg)
  
    # MQTT Connection
    def mqtt_connect(self):
        self.draw_string('Connecting to\n%s \nMQTT Broker'%(settings.mqtt_server))
        time.sleep(1)
        self.client = MQTTClient(client_id, settings.mqtt_server, keepalive=60)
        self.client.set_callback(self.mqtt_cb)

        while True:
            try:
                self.client.connect()
                self.draw_string('Connected to\n%s \nMQTT Broker'%(settings.mqtt_server))
                time.sleep(2)
                break
            except OSError as e:
                self.draw_string("MQtt.\nFailed\nRetrying")
            time.sleep(5)
        topic="pico_w5500/%s/CMD" % settings.ID
        print(topic)
        self.client.subscribe(str.encode(topic))

    def internal_temperature(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_temp)<t_sleep):
            return
        self.last_temp=tc
        conversion_factor = 3.3 / (65535)
        sensor_temp = ADC(4)
        reading = sensor_temp.read_u16() * conversion_factor 
        temperature = 27 - (reading - 0.706)/0.001721
        res={}
        res["T"]=temperature
        topic_pub = 'pico_w5500/%s/rp2040' % settings.ID 
        tmsg=json.dumps(res)
        self.client.publish(topic_pub.encode("utf8"), tmsg.encode("utf8"))
        self.draw_string("RP2040\nProcessor\n%.1f C" % temperature)
        time.sleep(1)
    def bme_measurement(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_bme)<t_sleep):
            return
        self.last_bme=tc
        t, p, h = self.bme.read_hrvalues()
        res={}
        res["T"]=t
        res["P"]=p
        res["H"]=h
        topic_pub ='pico_w5500/%s/bme280' % settings.ID
        tmsg=json.dumps(res)
        self.client.publish(topic_pub.encode("utf8"), tmsg.encode("utf8"))
        self.draw_string("BME280\nP=%.1f hPa\nT=%.1f C\nHum=%.1f %%" % (p,t,h))
        time.sleep(1)
    def hih_measurement(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_hih)<t_sleep):
            return
        self.last_hih=tc
        h,t=self.hih.read_sensor()
        res={}
        res["T"]=t
        res["H"]=h
        topic_pub = 'pico_w5500/%s/hih' % settings.ID
        tmsg=json.dumps(res)
        self.client.publish(topic_pub.encode("utf8"), tmsg.encode("utf8"))
        self.draw_string("HIH81310\nT=%.1f C\nHum=%.1f %%" % (t,h))
        time.sleep(1)
    def genesys_status(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_genesys)<t_sleep):
            return
        self.last_genesys=tc
        st=self.genesys.process_message("STATUS")
        
        topic_pub = 'pico_w5500/%s/genesys' % settings.ID
        tmsg=json.dumps(st)
        self.client.publish(topic_pub.encode("utf-8"), tmsg.encode("utf8"))
        self.draw_string("genesys\n%.1f %.1f %.1f %.1f\nStatus %s" %
                         (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        time.sleep(1)
        #time.sleep(t_sleep)
    def zup_status(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_zup)<t_sleep):
            return
        self.last_zup=tc
        st=self.zup.process_message("STATUS")
        
        topic_pub = 'pico_w5500/%s/zup' % settings.ID
        tmsg=json.dumps(st)
        self.client.publish(topic_pub.encode("utf-8"), tmsg.encode("utf8"))
        self.draw_string("zup\n%.1f %.1f %.1f %.1f\nStatus %s" %
                         (st["vset"],st["vout"],st["iset"],st["iout"],st["status"]))
        time.sleep(1)
        #time.sleep(t_sleep)
    def cpwplus_measurement(self,t_sleep):
        tc=time.time()
        if ((tc-self.last_cpwplus)<t_sleep):
            return
        self.last_cpwplus=tc
        st=self.cpwplus.process_message("STATUS")
        topic_pub = 'pico_w5500/%s/cpwplus' % settings.ID
        
        tmsg=json.dumps(st)
        self.client.publish(topic_pub.encode("utf8"), tmsg.encode("utf8"))
        self.draw_string("CPWPLUS\nNet Weight %.2f kg" % (st["net"]))
        time.sleep(1)
def main():
    pm=PmPico()
    pm.mqtt_connect()
    it=0
    while True:
        if settings.useTemp:
            pm.client.check_msg()
            pm.internal_temperature(2)
        if settings.useBME:
            pm.client.check_msg()
            pm.bme_measurement(10)
        if settings.useHIH:
            pm.client.check_msg()
            pm.hih_measurement(10)
        if settings.useGenesys:
            pm.client.check_msg()
            pm.genesys_status(10)
        if settings.useZup:
            pm.client.check_msg()
            pm.zup_status(10)
        if settings.useCpwplus:
            pm.client.check_msg()
            pm.cpwplus_measurement(10)
        #pm.draw_string("Next iteration \n %d" % it)
        it=it+1
        #pm.client.check_msg()
        time.sleep_ms(100)
    pm.client.disconnect()

if __name__ == "__main__":
    main()


