#!/usr/bin/env python3
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String,Float,Unicode
from spyne.protocol.json import JsonDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication
from spyne.model.complex import ComplexModelMeta
import monitor as mon
import os,sys,json
import socket
import time
_wdd=None
_wjobc=None
_sconf=None
class MyApplication(Application):
    def createAccess(self):    
        global _wdd
        global _sconf
        if (_wdd!=None):
            del _wdd;
        _sconf=json.load(open("/etc/wmon.json"))
        if (not "broker" in _sconf.keys()):
            print("Missing broker configuration")
        sbroker=_sconf["broker"]
        _wdd=mon.monitor(sbroker["host"],sbroker["port"],sbroker["session"])
        _wdd.Connect()
        _wdd.ListTopics()

        if ("mongo" in _sconf.keys()):
            os.environ["MGDBMON"]=_sconf["mongo"]
            _wdd.connectMongo()
        # stop all monitoring loops
        # restart loop with correct period
        
        _wdd.loop()
                        
        print('Wmon is running')

    def call_wrapper(self, ctx):
        try:
            return super(MyApplication, self).call_wrapper(ctx)

        except KeyError:
            raise ResourceNotFoundError(ctx.in_object)

class wddService(ServiceBase):
    @srpc(_returns=Iterable(String))
    def STOP_LOOP():
        global _wdd
        global _sconf
        if (_wdd!=None and _sconf!=None):
            for x in _sconf["tasks"]:
                logging.info("send END command to "+str(x))
                _wdd.sendCommand(x["name"],"END",{})
            yield 'STOP_LOOP done'
        else:
            yield " No slow control access"
    @srpc(_returns=Iterable(String))
    def START_LOOP():
        global _wdd
        global _sconf
        if (_wdd!=None and _sconf!=None):
            for x in _sconf["tasks"]:
                logging.info("send BEGIN command to "+str(x))
                _wdd.sendCommand(x["name"],"BEGIN",{"period":x["period"]})
            yield 'START_LOOP done'
        else:
            yield " No slow control access"




    
    @srpc(String,_returns=Iterable(String))
    def LV_ON(hw):
        global _wdd
        if (_wdd!=None):
            _wdd.LV_ON(hw)
            yield 'LV is ON for %s' % hw
        else:
            yield " No slow control access"

    @srpc(String,_returns=Iterable(String))
    def LV_OFF(hw):
        global _wdd
        if (_wdd!=None):
            _wdd.LV_OFF(hw)
            yield 'LV is OFF for %s' % hw
        else:
            yield " No slow control access"
    @srpc(_returns=Iterable(String))
    def UPDATE():
        global _wdd
        if (_wdd!=None):
            _wdd.UpdateInfos()
            yield 'Update required for all hardware'
        else:
            yield " No slow control access"

    @srpc( UnsignedInteger,_returns=Iterable(String))
    def ZupInfo(npmax):
        global _wdd
        if (_wdd!=None):
            yield _wdd.zupInfo(npmax)
        else:
            yield " No slow control access"
    @srpc( UnsignedInteger,_returns=Iterable(String))
    def BMPInfo(npmax):
        global _wdd
        if (_wdd!=None):
            yield _wdd.bmpInfo(npmax)
        else:
            yield " No slow control access"

    @srpc(UnsignedInteger,_returns=Iterable(String))
    def WienerInfo(npmax):
        global _wdd
        if (_wdd!=None):
            yield _wdd.WienerInfo(npmax)
        else:
            yield " No slow control access"

    @srpc( String, String, Float, _returns=Iterable(String))
    def HV_VSET(hardware,channels,vset):
        global _wdd
        if (_wdd!=None):
            jchannels=json.loads(channels)
            yield _wdd.HV_VSET(jchannels,vset,hw=hardware)
        else:
            yield " No slow control access"
    @srpc( String, String, Float, _returns=Iterable(String))
    def HV_ISET(hardware,channels,iset):
        global _wdd
        if (_wdd!=None):
            jchannels=json.loads(channels)
            yield _wdd.HV_ISET(jchannels,iset,hw=hardware)
        else:
            yield " No slow control access"
            

    @srpc( String, String,_returns=Iterable(String))
    def HV_ON(hardware,channels):
        global _wdd
        if (_wdd!=None):
            jchannels=json.loads(channels)
            yield _wdd.HV_ON(jchannels,hw=hardware)
        else:
            yield " No slow control access"
            
    @srpc( String, String,_returns=Iterable(String))
    def HV_OFF(hardware,channels):
        global _wdd
        if (_wdd!=None):
            jchannels=json.loads(channels)
            yield _wdd.HV_OFF(jchannels,hw=hardware)
        else:
            yield " No slow control access"

    @srpc( String, String,_returns=Iterable(String))
    def HV_CLEAR(hardware,channels):
        global _wdd
        if (_wdd!=None):
            jchannels=json.loads(channels)
            yield _wdd.HV_CLEAR(jchannels,hw=hardware)
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
    application = MyApplication([wddService], 'spyne.examples.hello.http',
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
    application.createAccess()
    # Now that we have our application, we must wrap it inside a transport.
    # In this case, we use Spyne's standard Wsgi wrapper. Spyne supports 
    # popular Http wrappers like Twisted, Django, Pyramid, etc. as well as
    # a ZeroMQ (REQ/REP) wrapper.
    wsgi_application = WsgiApplication(application)

    # More daemon boilerplate
    
    server = make_server(socket.gethostname(), 18018, wsgi_application)

    logging.info("listening to %s:%d" % (socket.gethostname(), 18018))
    ##logging.info("wsdl is at: http://lyosdhcal12:8100/?wsdl")

    server.serve_forever()

