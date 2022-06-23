#!/usr/bin/env python3
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String,Float,Unicode
from spyne.protocol.json import JsonDocument
from spyne.protocol.xml import XmlDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication
from spyne.model.complex import ComplexModelMeta
import monitor as dmon
import os,sys,json
import socket
import time
_wdd=None
_wjobc=None
_wsl=None

class wddService(ServiceBase):
    @srpc(_returns=Iterable(String))
    def createAccess():    
        global _wdd
        sconf=json.load(open("/etc/wmon.json"))
        if (not "broker" in sconf.keys()):
            yield "Missing broker configuration"
        sbroker=sconf["broker"]
        _wdd=mon.monitor(sbroker["host"],sbroker["port"],sbroker["session"])
        _wdd.Connect()
        _wdd.ListTopics()

        if ("mongo" in sconf.keys()):
            os.environ["MGDBMON"]=sconf["mongo"]
            _wdd.connectMongo()
        # stop all monitoring loops
        
        _wdd.loop()
                        
        if (_wdd==None):
            _wdd=Ld.LDaq()
            yield 'Daq is created'
        else:
            yield 'Daq is already created'

    @srpc(_returns=Iterable(String))
    def available():    
        global _wdd
        global _wjobc
        global _wsl
        daq="NONE"
        job="NONE"
        slow="NONE"
        if (_wdd!=None):
            daq="CREATED"
        if (_wjobc!=None):
            job="CREATED"
        if (_wsl!=None):
            slow="CREATED"
        yield '{"DAQ":"%s","JOB":"%s","SLOW":"%s" }' % (daq,job,slow)

    @srpc( String, _returns=Iterable(String))
    def setParameters(name):
        global _wdd
        print "On a recu ",name
        if (name!=None):
            _wdd.setParameters(name)
            #sm=_wdd.msg()
            ss=_wdd.state()
            yield 'parameters set %s ' % (ss)
        else:
            yield 'Cannot Set'
    @srpc( String, _returns=Iterable(String))
    def setDBState(name):
        global _wdd
        print "On a recu ",name
        if (name!=None):
            _wdd.setDBState(name)
            ss=_wdd.state()
            yield 'DBState set %s (%s)' % (name,ss)
        else:
            yield 'Cannot Set DB'
    @srpc(String, _returns=Iterable(String))
    def downloadDB(name):
        global _wdd
        if (_wdd!=None):
            if (name!=None):
                _wdd.setDBState(name)
            _wdd.downloadDB()
            ss=_wdd.state()
            yield 'DBState download  (%s)' % (ss)
        else:
            yield 'Cannot download DB'

    @srpc( _returns=String)
    def dbStatus():
       global _wdd
       if (_wjobc!=None):
           s=_wdd.dbStatus()
           yield s
       else:
           yield "No job control found" 


    @srpc( UnsignedInteger, _returns=Iterable(String))
    def setControlRegister(value):
        global _wdd
        print "On a recu ",value
        if (value!=None):
            _wdd.setControlRegister(value)
            ss=_wdd.state()
            yield 'ControlReg set %x (%s)' % (value,ss)
        else:
            yield 'Cannot Set Control register'
    
    @srpc( _returns=Iterable(String))
    def Discover():
        global _wdd
        l=_wdd.process("DISCOVER");
        yield 'DIM DNS is browsed %s (%s)' % (l,_wdd.state())

    @srpc(String, _returns=String)
    def createJobControl(name):
        global _wjobc
        #print "On a recu ",name
        if (_wjobc==None):
            _wjobc=Ldimjc.DimJobInterface()
            _wjobc.loadJSON(name)
            yield 'JOB Control interface  created '
        else:
            yield 'JOB Control interface  already created '
        #print _wjobc
        #yield 'JOB Control interface  created '

    @srpc( _returns=String)
    def jobStatus():
       global _wjobc
       if (_wjobc!=None):
           _wjobc.status()
           time.sleep(2)
           _wjobc.list()
           s=_wjobc.processStatusList()
           yield s
       else:
           yield "No job control found" 

    @srpc( _returns=String)
    def jobKillAll():
       global _wjobc
       if (_wjobc!=None):
           _wjobc.clearAllJobs()
           yield "All jobs killed"
       else:
           yield "No job control found"

    @srpc( _returns=String)
    def jobStartAll():
       global _wjobc
       if (_wjobc!=None):
           print _wjobc
           _wjobc.startJobs("ALL")
           time.sleep(2)
           yield "All jobs started"
       else:
           yield "No job control found"

        
    @srpc( String,_returns=Iterable(String))
    def forceState(name):
        global _wdd
        print "On a recu ",name
        #print "et on appelle",_wdd
        _wdd.forceState(name)
        yield 'State is set to %s ' % name  

    @srpc( _returns=Iterable(String))
    def prepareServices():
        global _wdd
        l=_wdd.process("PREPARE");
        yield l
        #yield 'Services ready %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def status():
       global _wdd
       yield _wdd.status()
       
    @srpc( _returns=Iterable(String))
    def shmStatus():
       global _wdd
       yield _wdd.builderStatus()

    @srpc( _returns=Iterable(String))
    def triggerStatus():
       global _wdd
       yield _wdd.triggerStatus()

    @srpc( _returns=Iterable(String))
    def resetTrigger():
       global _wdd
       _wdd.resetTriggerCounters()
       yield "Trigger counters are reset"

    @srpc( _returns=Iterable(String))
    def pause():
       global _wdd
       _wdd.pauseTrigger()
       yield "Trigger is paused"
    @srpc( _returns=Iterable(String))
    def resume():
       global _wdd
       _wdd.resumeTrigger()
       yield "Trigger is resumed"
    @srpc( _returns=Iterable(String))
    def pauseEcal():
       global _wdd
       _wdd.pauseEcal()
       yield "Trigger is paused"
    @srpc( _returns=Iterable(String))
    def resumeEcal():
       global _wdd
       _wdd.resumeEcal()
       yield "Trigger is resumed"

    @srpc( UnsignedInteger, _returns=Iterable(String))
    def triggerSpillOn(nclock):
       global _wdd
       _wdd.triggerSpillOn(nclock)
       yield "Spill On set to %d clock" % nclock

    @srpc( UnsignedInteger, _returns=Iterable(String))
    def triggerBeam(nclock):
       global _wdd
       _wdd.triggerBeam(nclock)
       yield "Beam length set to %d clock" % nclock

    @srpc( UnsignedInteger, _returns=Iterable(String))
    def triggerSpillOff(nclock):
       global _wdd
       _wdd.triggerSpillOff(nclock)
       yield "Spill Off set to %d clock" % nclock


    @srpc( _returns=Iterable(String))
    def LVStatus():
       global _wdd
       yield _wdd.LVStatus()

    @srpc( _returns=Iterable(String))
    def state():
       global _wdd
       yield _wdd.state()


    @srpc( _returns=Iterable(String))
    def initialise():
       global _wdd
       l=_wdd.process("INITIALISE")
       yield l
       #yield 'SDHCAL is Initialised %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def configure():
       global _wdd
       l=_wdd.process("CONFIGURE")
       yield l

       #_wdd.configure()
       #yield 'SDHCAL is configured %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def start():
       global _wdd
       l=_wdd.process("START")
       yield l

       #_wdd.start()
       #yield 'Run is Started %s (%s)' % (_wdd.msg(),_wdd.state())


    @srpc( _returns=Iterable(String))
    def stop():
       global _wdd
       l=_wdd.process("STOP")
       yield l

       #_wdd.stop()
       #yield 'Run is stopped %s (%s)' % (_wdd.msg(),_wdd.state())


    @srpc( _returns=Iterable(String))
    def destroy():
       global _wdd
       l=_wdd.process("DESTROY")
       yield l

       #_wdd.destroy()
       #yield 'Daq is Destroyed %s (%s)' % (_wdd.msg(),_wdd.state())

    @srpc( _returns=Iterable(String))
    def LVOFF():
       global _wdd
       _wdd.LVOFF()
       yield 'Lv is Off'


    @srpc( _returns=Iterable(String))
    def LVON():
       global _wdd
       _wdd.LVON()
       yield 'Lv is On'

    @srpc(_returns=Iterable(String))
    def createSlowControl():    
        global _wsl
        if (_wsl==None):
            _wsl=dc.DimSlowControl()
            yield 'Slow Control is created'
        else:
            yield 'Slow Control is already created'
        
    @srpc(_returns=Iterable(String))
    def hvStatus():
        global _wsl
        if (_wsl!=None):
            _wsl.readChannel(99)
            yield _wsl.hvinfoCrate()
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def setReadoutPeriod(period):
        global _wsl
        if (_wsl!=None):
            _wsl.setReadoutPeriod(period)
            yield 'Period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def startStorage(period):
        global _wsl
        if (_wsl!=None):
            _wsl.startStore(period)
            yield 'Storage started with period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def startCheck(period):
        global _wsl
        if (_wsl!=None):
            _wsl.startCheck(period)
            yield 'HV check started with period set to %d' % period
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def stopStorage():
        global _wsl
        if (_wsl!=None):
            _wsl.stopStore()
            yield ' Storage stopped'
        else:
            yield " No slow control access"
            
    @srpc(_returns=Iterable(String))
    def stopCheck():
        global _wsl
        if (_wsl!=None):
            _wsl.stopCheck()
            yield ' HV check stopped'
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def PT():
        global _wsl
        if (_wsl!=None):
            yield '{"P":%f,"T":%f}' % (_wsl.pression(),_wsl.temperature())
        else:
            yield " No slow control access"


    @srpc(String,_returns=Iterable(String))
    def initialiseDB(account):
        global _wsl
        if (_wsl!=None):
            _wsl.initialiseDB(account)
            yield 'mySql initialise with %s' % account
        else:
            yield " No slow control access"

    @srpc(_returns=Iterable(String))
    def loadReferences():
        global _wsl
        if (_wsl!=None):
            _wsl.loadReferences()
            yield ' HV references load to the wiener crate'
        else:
            yield " No slow control access"


    @srpc(UnsignedInteger,Float, _returns=Iterable(String))
    def setVoltage(channel,V):
        global _wsl
        if (_wsl!=None):
            _wsl.setVoltage(channel,V)
            yield " channel %d set to %f " % (channel,V)
        else:
            yield " No slow control access"
    @srpc(UnsignedInteger,Float, _returns=Iterable(String))
    def setCurrentLimit(channel,I):
        global _wsl
        if (_wsl!=None):
            _wsl.setCurrentLimit(channel,I)
            yield " channel %d set current to %f " % (channel,I)
        else:
            yield " No slow control access"
        
    @srpc(UnsignedInteger, _returns=Iterable(String))
    def HVON(channel):
        global _wsl
        if (_wsl!=None):
            _wsl.HVON(channel)
            yield " channel %d is ON " % (channel)
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger, _returns=Iterable(String))
    def HVOFF(channel):
        global _wsl
        if (_wsl!=None):
            _wsl.HVOFF(channel)
            yield " channel %d is OFF " % (channel)
        else:
            yield " No slow control access"

            
if __name__=='__main__':
    # Python daemon boilerplate
    from wsgiref.simple_server import make_server
    logging.basicConfig(level=logging.INFO)

    # Instantiate the application by giving it:
    #   * The list of services it should wrap,
    #   * A namespace string.
    #   * An input protocol.
    #   * An output protocol.
    application = Application([wddService], 'spyne.examples.hello.http',
          # The input protocol is set as HttpRpc to make our service easy to
          # call. Input validation via the 'soft' engine is enabled. (which is
          # actually the the only validation method for HttpRpc.)
          in_protocol=HttpRpc(validator='soft'),

          # The ignore_wrappers parameter to JsonDocument simplifies the reponse
          # dict by skipping outer response structures that are redundant when
          # the client knows what object to expect.
          #out_protocol=XmlDocument()
          #out_protocol=YamlDocument(),
          out_protocol=JsonDocument(ignore_wrappers=False),
      )

    # Now that we have our application, we must wrap it inside a transport.
    # In this case, we use Spyne's standard Wsgi wrapper. Spyne supports 
    # popular Http wrappers like Twisted, Django, Pyramid, etc. as well as
    # a ZeroMQ (REQ/REP) wrapper.
    wsgi_application = WsgiApplication(application)

    # More daemon boilerplate
    
    server = make_server(socket.gethostname(), 8200, wsgi_application)

    logging.info("listening to %s:%d" % (socket.gethostname(), 8200))
    logging.info("wsdl is at: http://lyosdhcal12:8100/?wsdl")

    server.serve_forever()

