from __future__ import absolute_import
from __future__ import print_function
import serviceAccess as sac
import session
# import MongoJob as mg
import json
import time
import os
from transitions import Machine, State


class daqControl:
    def __init__(self, config_file):
        self.config_file = config_file
        # parse config file
        self.session = session.create_session(config_file)
        # DB access
        # DAQ PART
        self.daq_answer = None
        self.daqfsm = Machine(model=self, states=[
                              'CREATED', 'INITIALISED', 'CONFIGURED', 'RUNNING', 'CONFIGURED'], initial='CREATED')
        self.daqfsm.add_transition(
            'initialise', 'CREATED', 'INITIALISED', after='daq_initialising', conditions='isConfigured')
        self.daqfsm.add_transition('configure', [
                                   'INITIALISED', 'CONFIGURED'], 'CONFIGURED', after='daq_configuring', conditions='isConfigured')
        self.daqfsm.add_transition(
            'start', 'CONFIGURED', 'RUNNING', after='daq_starting', conditions='isConfigured')
        self.daqfsm.add_transition(
            'stop', 'RUNNING', 'CONFIGURED', after='daq_stopping', conditions='isConfigured')
        self.daqfsm.add_transition(
            'destroy', 'CONFIGURED', 'CREATED', after='daq_destroying', conditions='isConfigured')

        self.stored_state = self.getStoredState()

    def fsmStatus(self):
        x = self.stored_state
        # print(" FSM Status:",x["name"],x["version"],x["location"],time.ctime(x["time"]),x["job"],x["daq"])

    def getStoredState(self):
        pl = self.session.pns_session_list(req_session=self.session.name())
        print(pl)
        if ("REGISTERED" in pl):
            if (pl["REGISTERED"] !=None):
                for x in pl["REGISTERED"]:
                    print(x)
                    if (x.split(':')[0] == self.session.name()):
                        return x.split(":")[1]

        print("On cree le state ")
        self.to_CREATED()
        self.session.pns_session_update(self.state)
        self.stored_state=self.state
        return "CREATED"

    def storeState(self):
        # register to PNS/SESSION
        print("Storing state ",self.state)
        pl=self.session.pns_session_update(self.state)
        print(pl)
        self.stored_state=self.getStoredState()
        self.fsmStatus()
        return

    def isConfigured(self):
        self.getStoredState()
        return True
    
    def updateInfo(self,printout,vverbose):
        self.session.Print(versbose)
    def processCommand(self,cmd,appname,param):
        r=self.session.commands(cmd,appname,param)
        return json.dumps(r)
  
