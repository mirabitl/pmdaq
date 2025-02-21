

import threading
import time
import logging
import sys
import time
import socket
import random
import os
import json
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("/tmp/febv2debug%d.log" % os.getpid(), mode='w')  # ,
        # logging.StreamHandler()
    ]
)


class dummy_socket:
    """The FEBV2 setup interface class.
    It hides access to MINIDAQ driver classes and implements basics function as
    init,configure,start,stop ....
    """
    def __init__(self,verbose=False):
        """
        Object creation

        Args:
            verbose (bool): False by default, tune to true for debug printout
        """
        self.detectorId = 201
        self.sourceId = 0xdead
        self.running = False
        self.tokens=100000
        self.throttle_low_limit=10000
        self.throttle_high_limit=20000
        self.run=0
        self.nacq=0
        self.gtc=0
        self.throttle_start=False
        self.state="CREATED"
        self.detectorId=201
        self.sourceId=10
        self.s_pause=False
    def initialise(self):
        """
        Initialise the setup. 
        It creates the access to the DB, access to FC7 and boot the FEB

        """
        logging.info("Dummy socket init")
        self.state="INITIALISED"
    def pause(self):
        self.s_pause=True
    def resume(self):
        self.s_pause=False
    
    def configure(self):
        """ Configure the setup

        It configures the FEB and prepare the FC7 for a run setting the orbit and trigger definition
        """
        self.tokens=100000
        self.throttle_low_limit=10000
        self.throttle_high_limit=20000
        self.run=0
        self.nacq=0
        self.state="CONFIGURED"
        logging.info("Dummy socket configured")
    def start(self,run=0):
        """ Start a run.

        It launches a thread that spy the data in the readout FIFO of the FEB
        If run is different from 0, it creates a new run (file) and writes the run header
        Args:
            run (int): By default it's 0 and shared memory is used. 
            
        """
        self.running= True
        self.producer_thread = threading.Thread(target=self.acquiring_data)
        self.producer_thread.start()
        logging.info("Daq is started")
        message = "Run number : "+str(self.run)
        logging.info(message)
        self.state="RUNNING"
    def status(self):
        """ returns the status of the acquisition
        Returns:
            A dictionnary object with status and event number
        """
        r={}
        if (self.running):
            r["state"]="running"
            r["run"]=self.run
            r["event"]=self.gtc
            r["tokens"]=self.tokens
        else:
            r["state"]="stopped"
            r["tokens"]=self.tokens
            
        return r  
    def acquiring_data(self):
        """ Acquisition thread

        While running, it loops continously and spy data in the FC7 readout fifo
        It writes data to disk or shared memory until running is false and the run stopped. 
        """
        UDP_IP = "192.168.100.1"
        UDP_PORT = 8765
        sock = socket.socket(socket.AF_INET, # Internet
                             socket.SOCK_DGRAM) # UDP


        M0=bytearray(1472)
        t0=time.time();
        for i in range(20,1400):
            M0[i]=i%255


        self.nacq=0
        self.gtc=0
        self.offset=0
        self.elen=0
        self.rlen=0
        vs=[random.randrange(1,248,1) for _ in range(100000)]
        while (self.running):
            if (self.throttle()):
                continue
            if (self.s_pause):
                time.sleep(0.2)
                continue
            if (self.offset==0):
                #self.elen=random.randrange(1,248)*1472
                self.elen=vs[self.gtc%100000]*1472
                self.gtc=self.gtc+1
                self.rlen=self.elen
            self.nacq=self.nacq+1
            M0[:4] = self.nacq.to_bytes(4, 'big')
            M0[4:8] = self.offset.to_bytes(4, 'big')
            M0[8:12] = self.elen.to_bytes(4, 'big')
            M0[12:16] = self.gtc.to_bytes(4, 'big')
            M0[16:20] = self.tokens.to_bytes(4, 'big')
            self.rlen=self.rlen-1472
            self.offset=self.offset+1472
            if (self.rlen<1):
                self.offset=0
                #time.sleep(1)
            if (self.nacq%3000 == 1 and self.nacq>1):
                time.sleep(1 /1000000.0)
                t=time.time()
                MB=self.nacq*1472/1024/1024.
                dt=(t-t0)
                MBS=MB/dt
                logging.info(f"Event {self.gtc} packet {self.nacq} MegaBytes {MB:.2f} Debit Mbit/s {MBS*8:.2f}")
            rs=sock.sendto(M0, (UDP_IP, UDP_PORT))
            self.tokens=self.tokens-1
        logging.info("Thread %d: finishing", self.run)
    def throttle(self):
        if (self.offset!=0):
            return False
        if (self.tokens<self.throttle_low_limit):
            self.throttle_start=True
            return True
        if (self.throttle_start and (self.tokens<self.throttle_high_limit)):
            return True
        return False
    def stop(self):
        """ Stop the run

        It sets running to false, wait for the thread to stop and disable FEB and FC7  acquisition
        """
        self.running= False
        self.producer_thread.join()
        logging.info("Daq is stopped")

    
