import json
import MongoHR2 as mg



def apply(fname,state,version,comment="none"):
    sdb=mg.HR2Instance()
    sdb.download(state,version)
    f=open(fname)
    sj=json.load(f)
    #print(sj)
    dn=["droite","centre","gauche"]
    for d in range(3):
        dif=sj["info"]["difs"][d]
        lasic=sj[dn[d]]
        for asic in range(1,49):
            v_g=lasic[asic]["gain"]
            print(dif,asic)
            for ip in range(64):
                if (v_g[ip]==255):
                    v_g[ip]=128
                sdb.setPadGain(dif,asic,ip,v_g[ip])
            print(v_g)
    sdb.uploadChanges(state,comment)

def maskchannel(dif,asic,pad,state,version,comment="none"):
    sdb=mg.HR2Instance()
    sdb.download(state,version)
    sdb.setChannelMask(dif,asic,pad,0)
    
    sdb.uploadChanges(state,comment)
            
