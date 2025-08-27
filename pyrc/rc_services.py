import json
import rc_request
import rc_pns

def create_app_access(pns_string):
     """! Create a service access from a PNS LIST answer
     @param pns_string PNS answer
     @ return A service access
     """
     o=rc_pns.strip_pns_string(pns_string)
     return app_access(o.host,o.port,o.session,o.name,o.instance)
    

class app_access:
    def __init__(self, vhost, vport,vsession,vname,vinstance):
        """!
        @brief Handle all fsmw instance application definition, parameters and methods

        It handles the state and the path of the fsmw and
        it has also  a list of
            -services
            -commands
            -transitions
            -allowed transitions

        @param vhost(str) Host name
        @param vport(int) Hots port
        @param vsession(str) Session name
        @param vname(str) Plugin name
        @param vinstance(int) Instance number 
        """
        self.pns=rc_pns.pns_access()
        ## Host name
        self.host = vhost
        ## Host port
        self.port = vport
        ## Session name
        self.session = vsession
        ## plugin name
        self.name = vname
        ## Instance number
        self.instance = vinstance
        ## PMDAQ URL
        self.url = "http://%s:%d" % (vhost, vport)
        ## State 
        self.state="VOID"
        ## Services path
        self.path="/%s/%s/%d/" % (vsession,vname,vinstance)
        self.state=self.pns.get_app_state(self.path)
        ## List of services
        self.services=[]
        ##List of commands
        self.commands=[]
        ## List of allowed transitions
        self.allowed=[]
        ## List of Transitions
        self.transitions=[]
        ## Parameters
        self.params=None
        if (self.state !="VOID" and self.state!="DEAD"):
            self.get_app_info()

    def get_app_info(self):
        """!
        Fill state, commands transitions and parameter with INFO and PARAMS commands
        """
        r_services=rc_request.executeCMD(self.host,self.port,"/SERVICES",{})
        #print(r_services)
        if (type(r_services) is bytes):
            r_services=r_services.decode("utf-8")
        services=json.loads(r_services);
        if ("http_error" in services):
            return
        #print(services)
        #print(services)
        for x in services:
            v=x.split(self.path)
            #print(len(v),v,self.services)
            if (len(v)==2):
                self.services.append(v[1])
        if (len(self.services)<3):
            print("Not a FSMW\n")
            return
        # Get info
        r_info=rc_request.executeCMD(self.host,self.port,"%sINFO" % self.path,{})
        if (type(r_info) is bytes):
            r_info=r_info.decode("utf-8")
        info=json.loads(r_info);
        if ("http_error" in info):
            return

        #print(info)
        # test STATE
        if (info["STATE"]!=self.state):
            print("PNS incoherency PNS=%s Process=%s" % (info["STATE"],self.state))
            return
        # Store available commads, transitions and allowed
        self.commands=[]
        self.allowed=[]
        self.transitions=[]
        for x in info['COMMANDS']:
            v=x.split("/")
            self.commands.append(v[len(v)-1])
        for x in info['TRANSITIONS']:
            v=x.split("/")
            self.transitions.append(v[len(v)-1])
        for x in info['ALLOWED']:
            v=x.split("/")
            self.allowed.append(v[len(v)-1])
        #Get parameter
        r_params=rc_request.executeCMD(self.host,self.port,"%sPARAMS" % self.path,{})
        if (type(r_params) is bytes):
            r_params=r_params.decode("utf-8")
        jpar=json.loads(r_params);
        if ("http_error" in jpar):
            return
        self.params=jpar
        #print(params)
    def register(self,param_s=None):
        """!
        Call the REGISTER command to the pmdaq daemon to create the plugin's services
        @param param_s Additional parameters if any 
        """
        if (self.state=='VOID' or self.state=='DEAD'):
            par={}
            par["session"]=self.session
            par["name"]=self.name
            par["instance"]=self.instance
            par["params"]=""
            if (param_s!=None):
                par["params"]=param_s
            r_services=rc_request.executeCMD(self.host,self.port,"/REGISTER",par)
            self.state=self.pns.get_app_state(self.path)
            if (self.state !="VOID"):
                self.get_app_info()
    def remove(self,par=None):
        """!
        Call the REMOVE command to the pmdaq daemon to remove the plugin's services
        @param par Additional parameters if any 

        """
        if (self.state!='VOID'):
            par={}
            par["session"]=self.session
            par["name"]=self.name
            par["instance"]=self.instance
            print("Sending remove",par)
            r_services=rc_request.executeCMD(self.host,self.port,"/REMOVE",par)
            self.state=self.pns.get_app_state(self.path)
            print("Removed state is %s \n" % self.state)

    def update_access_info(self):
        """!
        Reload all informations from PNS and from the plugin INFO as well
        """
        self.state=self.pns.get_app_state(self.path)
        self.services=[]
        self.commands=[]
        self.allowed=[]
        self.transitions=[]
        self.params=None
        if (self.state !="VOID"):
            self.get_app_info()

    def allInfos(self):
        """!
        Store in a dictionnary state,parameters and services name
        @return The dictionnary
        """
        jdict={}
        jdict['state']=self.state
        jdict['params']=self.params
        jdict['services']=self.services
        return jdict
    def isBaseApplication(self):
        """!
        Check the service is a fsmw
        @return Existence of PARAMS command (Should be true) 
        """
        return "PARAMS" in self.commands

    def sendCommand(self, name, content):
        """!
        Send a command to the plugin service
        @param name Command(service) Name
        @param content CGI parameters of the command
        @return The string answer 
        """
        self.update_access_info()
        isValid = name in self.commands
        if (not isValid):
            return '{"answer":"invalid command ","status":"FAILED"}'
        rep=rc_request.executeCMD(self.host,self.port,"%s%s" % (self.path,name),content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")
        return rep
    def sendTransition(self, name, content):
        """!
        Send a transition to the plugin service. The transition is checked to be in the ALLOWED list before beeing sent
        @param name Transition(service) Name
        @param content CGI parameters of the command
        @return The string answer  or a FAILED message
        """
        self.update_access_info()
        #print "Send Transition",self.procInfos
        isValid = False
        isValid = name in self.allowed
        if (not isValid):
            return '{"answer":"invalid transition","status":"FAILED"}'

        rep=rc_request.executeCMD(self.host,self.port,"%s%s" % (self.path,name),content)
        if (type(rep) is bytes):
            rep=rep.decode("utf-8")

        # update state (published is asynchronous)
        #time.sleep(10000/1000000.0)
        self.update_access_info()
        #print("New State is ",self.state)
        return rep

    def print(self, vverb):
        """!
        Print out of all service informations
        @param vverb verbose printout tag
        @return Non if vverb true or a dictionnary of all informations
        """
        if (vverb):
            print("FSM is %s on %s,  Service %s"  % (self.state, self.url, self.path))
            # print COMMAND and TRANSITION
            s= " \t Commands \t"
            for x in self.commands:
                s=s+x+" "
            print(s)
            s= " \t Transitions \t"
            for x in self.transitions:
                s=s+x+" "
            print(s)
            s= " \t Allowed \t"
            for x in self.allowed:
                s=s+x+" "
            print(s)
            print("\t Parameters")
            for k, v in self.params.items():
                print("\t \t", k, v)
        else:
            print("FSM is %s on %s,  Service %s"  % (self.state, self.url, self.path))
            r={}
            r["state"]=self.state
            r["url"]=self.url
            r["path"]=self.path
            r["commands"]=self.commands
            r["transitions"]=self.transitions
            r["allowed"]=self.allowed
            r["paramss"]=self.params
            return r
