from lxml import etree
import MongoHR2 as mg
s=mg.HR2Instance()
s.createNewState("Dome_42chambres_debug")
tree=etree.parse("./Dome_42chambres_Reference_v4_163.xml")

vtag=[]
for x in tree.xpath("ASIC"):
    for y in x.xpath("*"):
        if (y.tag in vtag):
            continue
        vtag.append(y.tag)
        print(y.tag,y.text)

vtag=['DIF_ID','B0', 'B1', 'B2', 'CLKMUX', 'CMDB0FSB1', 'CMDB0FSB2', 'CMDB0SS', 'CMDB1FSB1', 'CMDB1FSB2', 'CMDB1SS', 'CMDB2FSB1', 'CMDB2FSB2', 'CMDB2SS', 'CMDB3FSB1', 'CMDB3FSB2', 'CMDB3SS', 'DACSW', 'DIF_ID', 'DISCRI0', 'DISCRI1', 'DISCRI2', 'DISCROROR', 'ENABLED', 'ENOCCHIPSATB', 'ENOCDOUT1B', 'ENOCDOUT2B', 'ENOCTRANSMITON1B', 'ENOCTRANSMITON2B', 'ENTRIGOUT', 'EN_OTAQ', 'HEADER', 'NUM', 'OTABGSW', 'OTAQ_PWRADC','PWRONBUFF', 'PWRONFSB0', 'PWRONFSB1', 'PWRONFSB2', 'PWRONPA', 'PWRONSS', 'PWRONW', 'QSCSROUTSC', 'RAZCHNEXTVAL', 'RAZCHNINTVAL', 'RS_OR_DISCRI', 'SCON', 'SEL0', 'SEL1', 'SELENDREADOUT', 'SELSTARTREADOUT', 'SMALLDAC', 'SW100F0', 'SW100F1', 'SW100F2', 'SW100K0', 'SW100K1', 'SW100K2', 'SW50F0', 'SW50F1', 'SW50F2', 'SW50K0', 'SW50K1', 'SW50K2', 'SWSSC', 'TRIG0B', 'TRIG1B', 'TRIG2B', 'TRIGEXTVAL']
print(vtag)
v=input()
adif={}
for x in tree.xpath("ASIC"):
    vd={}
    for y in x.xpath("*"):
        if (not y.tag in vtag):
            continue
        vd[y.tag]=int(y.text)
    dif=(vd["DIF_ID"])
    header=(vd["HEADER"])
    if (not dif in adif.keys()):
        adif[dif]={}
    if (not header in adif[dif].keys()):
        adif[dif][header]=vd

for d,v in adif.items():
    print("DIF : ",d,len(v))
    s.addBoard(d,len(v))
    vare=input()
    for x,y in v.items():
        print("ASIC : ",x)
        print(y)
        #print(adif)
        for t,vt in y.items():
            s.changeParam(t,vt,d,x)
            print("Changing %s %d for DIF %d ASIC %d \n" % (t,vt,d,x))
s.setPowerPulsing()
s.uploadNewState("Dome 42 chambre from state 163")
for x in tree.xpath("ASIC"):
    header=0
    dif=0
    enabled=0
    b0=0
    for y in x.xpath("HEADER"):
        header=int(y.text)
    for y in x.xpath("DIF_ID"):
        dif=int(y.text)
    for y in x.xpath("ENABLED"):
        enabled=int(y.text)
    for y in x.xpath("B0"):
        b0=int(y.text)
    for y in x.xpath("B1"):
        b0=int(y.text)
    for y in x.xpath("B2"):
        b0=int(y.text)
    print(dif,header,enabled,b0)
    if (enabled==0):
        v=input()
