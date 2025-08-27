
import os
import rc_request
import json

def strip_pns_string(pns_string):
    """!
    Analyse answer of PNS  to return an object of information on the service

    @param pns_string PNS answer
    @return Directory {'host':host, 'port':port,'path':path,'session':session,'name':name,'instance':instance,'state':state}

    """
    state=pns_string.split("?")[1]
    v=pns_string.split("?")[0].split(":")
    host=v[0];
    port=int(v[1]);
    path =v[2]
    vp=path.split("/")
    session=vp[1]
    name=vp[2]
    instance=int(vp[3])
    o = type('serviceAccessDescription', (object,), 
                {'host':host, 'port':port,'path':path,'session':session,'name':name,'instance':instance,'state':state})()
    #print(o)
    #print("=>",o.host,o.port,o.path,o.session,o.name,o.instance,o.state)
    return o

class pns_access:
    def __init__(self,vport=8888):
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
        self.host=os.getenv("PNS_NAME","NONE")
        if (self.host == "NONE"):
            print("The ENV varaible PNS_NAME mut be set")
            exit(0)
        ## Host port
        self.port = vport
            

    def print_app_registered(self):
        """!
        Print out informations of all REGISTERED services 
        """
        
        r_pns_list=rc_request.executeCMD(self.host,self.port,"/PNS/LIST",{})
        if (type(r_pns_list) is bytes):
            r_pns_list=r_pns_list.decode("utf-8")
        pns_list=json.loads(r_pns_list)
        if ("http_error" in r_pns_list):
            return
        #print(pns_list)
        if ("REGISTERED" in pns_list):
            #print(pns_list["REGISTERED"])
            if ( pns_list["REGISTERED"]!=None):
                for x in pns_list["REGISTERED"]:
                    o =self.strip_pns_string(x)
                    print("=>",o.host,o.port,o.path,o.session,o.name,o.instance,o.state)        
    def get_app_state(self,s_path):
        """! Find the current state of the service from a PNS/LIST request
        """
        r_pns_list=rc_request.executeCMD(self.host,self.port,"/PNS/LIST",par)
        if (type(r_pns_list) is bytes):
            r_pns_list=r_pns_list.decode("utf-8")
        pns_list=json.loads(r_pns_list)
        if ("http_error" in r_pns_list):
            return
        #print(pns_list)
        if ("REGISTERED" in pns_list):
            #print(pns_list["REGISTERED"])
            if ( pns_list["REGISTERED"]!=None):
                for x in pns_list["REGISTERED"]:
                    o =self.strip_pns_string(x)
                    #print( iho,ipo,ipa,ises,ina,iin)
                    #st=x.split('?')[0].split(':')[2]
                    if (o.path==s_path):
                        return o.state
        return "NOTREGISTERED"    

    def list(self,req_session="NONE"):
        """! Call PNS/LIST for a session name
        @param req_session Session name
        @return Python object built from PNS/LIST answer
        """
        par={}
        par["session"]=req_session
        #print(par)
        pl=json.loads(rc_request.executeCMD(self.host,self.port,"/PNS/LIST",par))
        return pl
    
    def purge(self):
        pl=json.loads(rc_request.executeCMD(self.host,self.port,"/PNS/PURGE",{}))
        return pl
    def session_list(self,req_session="NONE"):
        """! Call PNS/SESSION/LIST for a session name
        @param req_session Session name
        @return Python object built from PNS/SESSION/LIST answer
        """
        pl=json.loads(rc_request.executeCMD(self.host,self.port,"/PNS/SESSION/LIST",{"session":req_session}))
        return pl
    def session_update(self,params):
        pl=json.loads(rc_request.executeCMD(self.host,self.port,"/PNS/SESSION/UPDATE",params))
        return pl
    def session_purge(self):
        pl=json.loads(rc_request.executeCMD(self.host,self.port,"/PNS/SESSION/PURGE",{}))
        return pl
            