
import os
import json
import six
import rc_services as sac
import json
import rc_request
import rc_pns
def create_session(config):
    """!
    Parse the configuration files, build the corresponding serviceAccess and REGISTER all plugins if needed
    @param config The configuration file
    @return The session_access object built with the configuration dictionnary
    """
    j_sess=json.loads(open(config).read())
    #print(j_sess)
    vsession=j_sess['session']
    if ('apps' in j_sess):
        for x in j_sess['apps']:
            a=sac.app_access(x['host'],x['port'],vsession,x['name'],x['instance'])
            if (a.state=='VOID'):
                par={}
                if ('params' in x):
                    par=x['params']
                a.register(par)
    return session_access(vsession)
                
            
class session_access:
    def __init__(self,vsession):
        """!
        Handle all application of the session. The applictaion are stored in a map of pluggin names each entry conatining a list of serviceAccess to each instance of the plugin
        @param vsession The session dictionnary
        """
        ## Name of the session
        self.session = vsession
        ## Directory of applications
        self.apps={}
        ## Name of the PNS host
        self.pns=rc_pns.pns_access()
        pl= self.pns.list()
        #json.loads(sac.executeCMD(self.pns_host,8888,"/PNS/LIST",{}))
        #.decode("utf-8"))
        if ("REGISTERED" in pl):
            if ( pl["REGISTERED"]!=None):
                for x in pl["REGISTERED"]:
                    o =sac.strip_pns_string(x)
                    if (o.session!=self.session):
                        continue
                    #print( iho,ipo,ipa,ises,ina,iin)
                    #st=x.split('?')[0].split(':')[2]
                    if (o.name in self.apps):
                        self.apps[o.name].append(sac.app_access(o.host,o.port,o.session,o.name,o.instance))
                    else:
                        self.apps[o.name]=[]
                        self.apps[o.name].append(sac.app_access(o.host,o.port,o.session,o.name,o.instance))
    def name(self):
        """! getter of session name"""
        return self.session

    def Print(self,verbose=False):
        """! Print out of all serviceAccess
        """
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            for y in app:
                y.print(verbose)
                
    def remove(self,obj_name=None):
        """! Call the REMOVE command for all services
        @param obj_name if not None remove only plugins with this name

        It first removes all plugins but the evb_builder
        It then removes the evb_builder plugins
        Then it purges the PNS, tag the PNS/SESSION as DEAD and purge the PNS/SESSION

        """
        # First skip event builder (ZMQ issue)
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            if (obj_name!=None and obj_name!=name):
                continue
            if (name == "evb_builder"):
                continue
            for y in app:
                y.remove()
        # Now remove evb_builder
        for name,app in six.iteritems(self.apps):
            print(self.session,"===> ",name)
            if (obj_name!=None and obj_name!=name):
                continue
            if (name != "evb_builder"):
                continue
            for y in app:
                y.remove()
        # clear PNS
        print("purging PNS")
        self.pns.purge()

        sac.executeCMD(self.pns_host,8888,"/PNS/PURGE",{})
        # removing session
        print("purging PNS/SESSION")
        self.pns.session_update({"session":self.name,"state":"DEAD"})
        self.pns.session_purge()
    def commands(self,cmd,obj_name,params=None):
        """! Send a command to a plugin group
        @param cmd Command name
        @param obj_name The plugin name
        @param params The CGI parameters
        @return Python object of the answer
        """
        rep={}
        for name,app in six.iteritems(self.apps):
            if (obj_name!=None and obj_name!=name):
                continue
            for y in app:
                r= y.sendCommand(cmd,params)
                rep[y.path]=json.loads(r)
        # clear PNS
        return rep
    def transitions(self,cmd,obj_name,params=None):
        """! Send a transition to a plugin group
        @param cmd Transition name
        @param obj_name The plugin name
        @param params The CGI parameters
        @return Python object of the answer
        """
        rep={}
        for name,app in six.iteritems(self.apps):
            if (obj_name!=None and obj_name!=name):
                continue
            for y in app:
                r= y.sendTransition(cmd,params)
                rep[y.path]=r
                #print(y.path,r)
                
        # clear PNS
        return rep

