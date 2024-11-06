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
        """!
        Abstract class instantiating a sessionAccess and a Transitions state Machine 
        
        STATES='CREATED', 'INITIALISED', 'CONFIGURED', 'RUNNING'
        TRANSITIONS='initialise  callback daq_initialising
                     'configure' callback daq_configuring
                     'start'     callback daq_starting
                     'stop'      callback daq_stoping
                     'destroy'   callback daq_destroying

        ALl the callbacks are defined in the inheriting classess 
 
        """
        ## Configuration file
        self.config_file = config_file
        # parse config file
        #print(self.config_file)
        ## session.sessionAccess 
        self.session = session.create_session(config_file)
        # DB access
        # DAQ PART
        self.daq_answer = None
        ## DAQ Finite State Machine (transitions.Machine)
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
        ## FSM state
        self.stored_state = self.getStoredState()

    def fsmStatus(self):
        """! Print the FSM status
        @deprecated
        """
        x = self.stored_state
        # print(" FSM Status:",x["name"],x["version"],x["location"],time.ctime(x["time"]),x["job"],x["daq"])

    def getStoredState(self):
        """!
        Get the session state from the PNS/SESSION/LIST

        If not known set it to CREATED and updates PNS/SESSION
        @return state name
        """
        pl = self.session.pns_session_list(req_session=self.session.name())
        #print(pl)
        if ("REGISTERED" in pl):
            if (pl["REGISTERED"] !=None):
                for x in pl["REGISTERED"]:
                    #print(x)
                    if (x.split(':')[0] == self.session.name()):
                        return x.split(":")[1]

        print("On cree le state ")
        self.to_CREATED()
        self.session.pns_session_update(self.state)
        self.stored_state=self.state
        return "CREATED"

    def storeState(self):
        """!
        Update the session state in PNS/SESSION list
        """
        # register to PNS/SESSION
        print("Storing state ",self.state)
        pl=self.session.pns_session_update(self.state)
        print(pl)
        self.stored_state=self.getStoredState()
        self.fsmStatus()
        return

    def isConfigured(self):
        """! 
        Get the stored state
        It can be redefined in inheriting class
        @return Allways true
        """
        self.getStoredState()
        return True
    
    def updateInfo(self,printout,vverbose):
        """! Print out session infos
        """
        self.session.Print(vverbose)
    def processCommand(self,cmd,appname,param):
        """!
        proxy for sessionAccess command
        @param cmd Command Name 
        @param appname Plugin name
        @param CGI parameters of the command
        @return String answer of the command
        """
        r=self.session.commands(cmd,appname,param)
        return json.dumps(r)
  
