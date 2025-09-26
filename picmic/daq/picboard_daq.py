#!/usr/bin/env python3
import logging
from spyne import Application, srpc, ServiceBase, Iterable, UnsignedInteger,String,Float,Unicode,Integer16
from spyne.protocol.json import JsonDocument
from spyne.protocol.csv import Csv
from spyne.protocol.http import HttpRpc
from spyne.server.wsgi import WsgiApplication
from spyne.model.complex import ComplexModelMeta
import  picboard_fsm as FSM
import os,sys,json
import socket
import time
_wdd=None
_wjobc=None
_sconf=None
class MyApplication(Application):
    """ Spyne base application class
    """
    def createAccess(self):
        """ Access creation

        No access to the daq is created here but in the CREATE method of the wddService
        """
        # global _wdd
        # if (_wdd!=None):
        #     del _wdd;
       
        # _wdd=FSM.febv2_fsm("/etc/febv2_fsm.json")
        # _wdd.setOrbitMode()
                        
        print('Light febv2 daq is running')

    def call_wrapper(self, ctx):
        """
        Wrapper
        """
        try:
            return super(MyApplication, self).call_wrapper(ctx)

        except KeyError:
            raise ResourceNotFoundError(ctx.in_object)

class wddService(ServiceBase):
    """ Main service class
    """
    @srpc(String,UnsignedInteger,_returns=Iterable(String))
    def CREATE(name,version):
        """ Create a febv2_fsm object

        Args:
            name (str): configuration file name or acquisition state name
            version (int): 0 if file is used or version number
        """
        global _wdd
        if (name[0]=='"'):
            name=name[1:len(name)-1]
        
        if (_wdd!=None ):
            del _wdd

        if (version!=0):
            os.environ["DAQSETUP"]="%s:%d" % (name,version) 
            _wdd=FSM.picboard_fsm()
        else:
            _wdd=FSM.picboard_fsm(config_file=name)
        status={}
        print("On est la",_wdd.state)
        status["STATUS"]=_wdd.state
        yield json.dumps(status)
        

        

    @srpc(_returns=Iterable(String))
    def INITIALISE():
        """ calls febv2_fsm initialise transition
        """
        global _wdd
        if (_wdd!=None ):
            
            _wdd.initialise()
            time.sleep(0.3)
            status={}
            status["STATUS"]=_wdd.state
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)

    @srpc(_returns=Iterable(String))
    def STATUS():
        """ Gte the febv2_setup status
        """
        global _wdd
        if (_wdd!=None ):
            #_wdd.status()
            time.sleep(0.3)
            status={}
            status["DETID"]=_wdd.setup.detectorId;
            status["SOURCEID"]=_wdd.setup.sourceId;
            status["STATUS"]=_wdd.setup.status()
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
    
    @srpc(_returns=Iterable(String))
    def CONFIGURE():
        """ calls febv2_fsm configure transition
        """
        global _wdd
        if (_wdd!=None ):
            _wdd.configure()
            time.sleep(0.3)
            status={}
            status["STATUS"]=_wdd.state
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
    @srpc(_returns=Iterable(String))
    def START():
        """ calls febv2_fsm start transition
        """
        global _wdd
        if (_wdd!=None ):
            _wdd.start()
            time.sleep(0.3)
            status={}
            status["STATUS"]=_wdd.state
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
    @srpc(_returns=Iterable(String))
    def STOP():
        """ calls febv2_fsm stop transition
        """
        global _wdd
        if (_wdd!=None ):
            _wdd.stop()
            time.sleep(0.3)
            status={}
            status["STATUS"]=_wdd.state
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
    @srpc(_returns=Iterable(String))
    def DESTROY():
        """ calls febv2_fsm destroy transition
        """
        global _wdd
        if (_wdd!=None ):
            _wdd.destroy()
            time.sleep(0.3)
            exit(0)
            status={}
            status["STATUS"]=_wdd.state
            del _wdd
            _wdd=None
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)

    @srpc( String, UnsignedInteger, _returns=Iterable(String))
    def CHANGEDB(statename,version):
        """ calls febv2_fsm changeDb method

        Args:
            statename (str): name of the state
            version (int): state version
        """
        global _wdd
        if (_wdd!=None):
            if (statename[0]=='"'):
                statename=statename[1:len(statename)-1]
            _wdd.changeDB(statename,version)
            status={}
            status["STATE"]=statename
            status["VERSION"]=version
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
    @srpc( String, UnsignedInteger, _returns=Iterable(String))
    def CHANGEPARAMS(pname,pval):
        """ calls febv2_fsm change_vth_time method

        Args:
            shift (int): VTH_TIME shift
        """
        global _wdd
        if (_wdd!=None):
            _wdd.change_params(pname,pval)
            status={}
            status["PARAMETER"]=pname
            status["VALUE"]=pval
            yield json.dumps(status)
        else:
            status={}
            status["STATUS"]="FAILED"
            yield json.dumps(status)
       
         
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
    
    server = make_server(socket.gethostname(), 29029, wsgi_application)

    logging.info("listening to %s:%d" % (socket.gethostname(), 29029))
    ##logging.info("wsdl is at: http://lyosdhcal12:8100/?wsdl")

    server.serve_forever()

