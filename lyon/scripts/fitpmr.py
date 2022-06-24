#!/usr/bin/python3
import sqlite3
from ROOT import TFile,TF1,TH1F,TProfile,TH2F,TCanvas,gStyle,TAttLine
#import ROOT.TH1
import os
import json
import math
import time
import numpy as np
from scipy import stats
import MongoJob
from array import array
_wdd=MongoJob.instance()


class prettyfloat(float):
    def __repr__(self):
        return "%0.2f" % self


defped=[31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31]
#defped=[30, 30, 34, 29, 32, 32, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 33, 27, 31, 29, 25, 35, 33, 9]
#defped=[39,38,42,37,41,40,46,0,43,48,37,49,0,0,0,0,0,0,0,0,39,39,36,35,41,35,40,34,32,44,43,34]

defped=[38,38,41,39,41,40,45,0,42,47,37,47,0,0,0,0,0,0,0,0,38,38,35,36,42,36,40,36,33,43,42,35]
def calP(p0,alt):
  return p0*(1-0.0065*alt/288.15)**5.255

def calV(V,P,T):
  #print 1-(0.2+0.8*P/990.*293./T)
  return  V/(0.2+0.8*P/990.*293./T)

def calValice(V,P,T):
  print(1-(1*P/990.*293./T))
  return  V/(1.0*P/990.*293./T)

def appAlice(P,T,Vwanted):
  return  Vwanted*(1.0*P/990.*293./T)


def calApp(V,P,T):
  print(1-(0.2+0.8*P/990.*293./T))
  return  V*(0.2+0.8*P/990.*293./T)

def allruns(ri,rf):
    fout=open("/home/mirabito/output/run%d_%d.csv" % (ri,rf) ,mode="w+")
    for r in range(ri,rf+1):
        rif= _wdd.runInfo(r,"TRICOT2M2",printout=False)
        if (rif==None):
            print("Unkonown runs",r)
            continue
        if (not "P" in rif.keys()):
            continue
        rep=gricpmr(r,interactive=False)
        if (rep!=None):
            fout.write(rep+"\\\ \n")
    fout.close()
def gricpmr(run,sub="/tmp/",interactive=True):
    rif= _wdd.runInfo(run,"TEST_CHAMBRE_PM",printout=False)
    if (rif==None):
        print("Unkonown runs",run)
        return None
    #print(rif)
    if ("P" in rif.keys()):
        P=rif["P"]
        T=rif["T"]
        vapp=int(rif["hv"]*1000)
        #veff=calValice(vapp,P,T)
        veff=rif["valice"]
        gas=""
        if ("HFO" in rif):
            gas="%4.1f_%4.1f_%4.1f" % (rif["HFO"],rif["CO2"],rif["SF6"])
        if ("TFE" in rif):
            gas="%4.1f_%4.1f_%4.1f" % (rif["TFE"],rif["CO2"],rif["SF6"])
        
    else:
        P=1002
        T=288
        vapp=7800
        veff=7800
        gas="97_2.7_2"
    c=TCanvas("test","test",800,800)
    f=TFile(sub+"histo%d_0.root" % run)
    hc=f.Get("/pmr/stat")
    if (hc == None):
        return None
    ntrg=hc.GetBinContent(2)
    nfound=hc.GetBinContent(12)

    
    
    hq=f.Get("/pmr/INT/qall")
    nl=0
    na=hq.GetEntries()
    for ib in range(1,hq.GetNbinsX()):
        if (hq.GetBinCenter(ib)>15):
            nl=nl+hq.GetBinContent(ib)
    hnh=f.Get("/pmr/INT/nhit")
    mnh=hnh.GetMean()
    nenh=hnh.GetEntries()
    nl10=0
    for ib in range(1,hnh.GetNbinsX()):
        if (hnh.GetBinCenter(ib)>10):
            nl10=nl10+hnh.GetBinContent(ib)

    fout=open("/home/mirabito/output/run%d.csv" % run ,mode="w+")
    print("%d &%d & %d & %5.1f & %5.1f & %s & %d & %d & %5.1f &  & %d & %5.1f & %5.2f & %d & %5.1f " % (run,vapp,veff,P,T,gas,int(ntrg),int(nfound),100.*nfound/ntrg,int(nl),100.*nl/na,mnh,nl10,100.*nl10/nenh))
    fout.write("%d &%d & %d & %5.1f & %5.1f & %s & %d & %d & %5.1f &  & %d & %5.1f & %5.2f & %d & %5.1f\n" % (run,vapp,veff,P,T,gas,int(ntrg),int(nfound),100.*nfound/ntrg,int(nl),100.*nl/na,mnh,nl10,100.*nl10/nenh))
    rep="%d &%d & %d & %5.1f & %5.1f & %s & %d & %d & %5.1f &  & %d & %5.1f & %5.2f & %d & %5.1f" % (run,vapp,veff,P,T,gas,int(ntrg),int(nfound),100.*nfound/ntrg,int(nl),100.*nl/na,mnh,nl10,100.*nl10/nenh)


    fout.close()
    
    c.Divide(2,3)
  
    c.cd(1);f.Get("/pmr/INT/nframe").Draw()
    c.cd(2);f.Get("/pmr/INT/nhit").Draw()
    c.cd(3);f.Get("/pmr/INT/nh0").Draw()
    c.cd(4);f.Get("/pmr/INT/nh1").Draw()
    c.cd(5);f.Get("/pmr/INT/nh2").Draw()
    c.cd(6);f.Get("/pmr/MaxTime").Draw()
    c.Draw()
    c.Update()
    c.SaveAs("~/output/run%d.pdf" % (run))
    if (interactive):
        v=input("Continue?")
    c.Clear()
    c.Divide(1,3)
    c.cd(1);f.Get("/pmr/ASIC/D90/XY").Draw("COLZ")
    c.cd(2);f.Get("/pmr/INT/D90/XY").Draw("COLZ")
    c.cd(3);hq.Draw();
    c.Draw()
    c.Update()
    c.SaveAs("~/output/run%d_pos.pdf" % (run))
    if (interactive):
        v=input("Continue?")

    return rep
def getdtc(run,chamber,sub=""):
  c=TCanvas()
  f82=TFile(sub+"histo%d_0.root" % run);
  r0=[]
  r1=[]
  for i in range(0,48):
    r0.append(0)
    r1.append(0)
  ht0=None
  ht1=None
  if (chamber==1):
      f82.cd("/Clusters/ch1")
      ht0=f82.Get("/Clusters/ch1/T0")
      ht1=f82.Get("/Clusters/ch1/T1")
      ht0.Draw()
      c.Modified()
      c.Update()
      val=input()
  if (chamber==2):
      f82.cd("/Clusters/ch2")
      ht0=f82.Get("/Clusters/ch2/T0")
      ht1=f82.Get("/Clusters/ch2/T1")

  scfit=TF1("scfit","gaus",-10.,10.)

  for i in range(0,48):
    #print "/run%d/Chamber%d/FEB/%d/Side0/channel%d" % (run,chamber,feb,i)
    hch=ht0.ProjectionY("ch",i,i+1)
    if (hch.GetEntries()>20):
        hch.Fit("scfit","Q","");
        c.cd()
        hch.Draw()
        c.Modified()
        c.Update()
        val=input()
        dtmean=scfit.GetParameter(1)
        dtres=scfit.GetParameter(2)
        r0[i]=dtmean
    else:
        r0[i]=0;
    hch1=ht1.ProjectionY("ch1",i,i+1)
    if (hch.GetEntries()>20):
        hch1.Fit("scfit","Q","");
        #c.cd()
        #hch.Draw()
        #c.Modified()
        #c.Update()
        #val=input()
        dtmean=scfit.GetParameter(1)
        dtres=scfit.GetParameter(2)
        r1[i]=dtmean
    else:
        r1[i]=0;

  r0 = map(prettyfloat, r0)
  print('"dt0":',r0)
  r1 = map(prettyfloat, r1)
  print('"dt1":',r1)

def drawtdc(run,sub="Histos/InTime/"):
    c=TCanvas()
    hmean=TH1F("hmean","Summary of mean distance ",1000,-20.,20.)
    hrms=TH1F("hrms","Summary of dispersion ",200,0,0.04)
    f82=TFile(sub+"histo%d_0.root" % run);
    f82.cd("/run%d/Raw/DT/" % run )
    for i in range(64):
        for j in range(i+1,64):
            hch=f82.Get("/run%d/Raw/DT/ch_%d_%d" % (run,i,j))
            if (hch==None):
                continue
            m=hch.GetMean()
            r=hch.GetRMS()
            hmean.Fill(m)
            hrms.Fill(r)
    c.cd()
    hmean.Draw()
    c.SaveAs("Dispersion_channel_to_channel_%d.png" % run)
    v=input()
    
    hrms.Draw()
    c.Update()
    c.SaveAs("RMS_channel_to_channel_%d.png" % run)
    v=input()
def getdt(run,chamber,feb,sub=""):
  c=TCanvas()
  f82=TFile(sub+"histo%d_0.root" % run);
  r=[]
  for i in range(0,49):
    r.append(0)
  for i in range(1,49):
    #print "/run%d/Chamber%d/FEB/%d/Side0/channel%d" % (run,chamber,feb,i)
    f82.cd("/run%d/InTime/Chamber%d/FEB/%d/Side0/" % (run,chamber,feb))
    hch=f82.Get("/run%d/InTime/Chamber%d/FEB/%d/Side0/channel%d" % (run,chamber,feb,i))
    #print hch
    if (hch!=None):
      #print i,hch.GetEntries(),hch.GetMean();
      if (hch.GetEntries()>25):
          if (hch.GetEntries()<100):
              hch.Rebin(2)
          if (hch.GetEntries()<50):
              hch.Rebin(2)
          nmax=0
          imax=0
          for ib in range(1,hch.GetNbinsX()):
              if (hch.GetBinContent(ib)>nmax):
                  nmax=hch.GetBinContent(ib)
                  imax=ib

          vmin=hch.GetBinCenter(imax)-7;
          vmax=hch.GetBinCenter(imax)+7;
          print(vmin,vmax,imax,nmax)
          scfit=TF1("scfit","gaus",vmin,vmax)
          hch.Fit("scfit","Q","",vmin,vmax);
          dtmean=scfit.GetParameter(1)
          dtres=scfit.GetParameter(2)
          print(i,hch.GetEntries(),hch.GetMean(),dtmean,dtres)
          c.cd()
          hch.Draw()
          c.Modified()
          c.Update()
          val=input()
          r[i]=dtmean
          if (abs(dtmean-hch.GetMean())>hch.GetRMS()):
              r[i]=hch.GetMean()
      continue
    
    f82.cd("/run%d/InTime/Chamber%d/FEB/%d/Side1/" % (run,chamber,feb))
    hch1=f82.Get("/run%d/InTime/Chamber%d/FEB/%d/Side1/channel%d" % (run,chamber,feb,i))
    #print hch1
    if (hch1!=None):
      #print i,hch1.GetEntries(),hch1.GetMean();
      if (hch1.GetEntries()>25):
          if (hch1.GetEntries()<100):
              hch1.Rebin(2)
          if (hch1.GetEntries()<50):
              hch1.Rebin(2)

          nmax=0
          imax=0
          for ib in range(1,hch1.GetNbinsX()):
              if (hch1.GetBinContent(ib)>nmax):
                  nmax=hch1.GetBinContent(ib)
                  imax=ib

          vmin=hch1.GetBinCenter(imax)-7;
          vmax=hch1.GetBinCenter(imax)+7;

          scfit=TF1("scfit","gaus",vmin,vmax)
          hch1.Fit("scfit","Q","",vmin,vmax);
          dtmean=scfit.GetParameter(1)
          dtres=scfit.GetParameter(2)
          print(i,hch1.GetEntries(),hch1.GetMean(),dtmean,dtres)
          c.cd()
          hch1.Draw()
          c.Modified()
          c.Update()
          val=input()

          r[i]=dtmean
          #hch1.GetMean()
          if (abs(dtmean-hch1.GetMean())>hch1.GetRMS()):
              r[i]=hch1.GetMean()

      continue

  #print r
  hfeb0=f82.Get("/run%d/InTime/Chamber%d/FEB/%d/Side0/DTall" % (run,chamber,feb))
  hfeb1=f82.Get("/run%d/InTime/Chamber%d/FEB/%d/Side1/DTall" % (run,chamber,feb))
  sfit=TF1("sfit","gaus",-150.,-100.)
  hfeb0.Fit("sfit","Q","",-150.,-100.);
  c.cd()
  hfeb0.Draw()
  c.Modified()
  c.Update()
  #val=input()
  print("dt0: %5.1f" % (sfit.GetParameter(1)))
  hfeb1.Fit("sfit","Q","",-150.,-100.);
  c.cd()
  hfeb1.Draw()
  c.Modified()
  c.Update()
  #val=input()
  print("dt1: %5.1f"  % sfit.GetParameter(1))
  
  r = map(prettyfloat, r)
  print('"dtc":',r)
def fitProfile(run,sel=92):
  f82=TFile("./histo%d_0.root" % run);
  #f82.cd("/run%d/TDC%d/LmAnalysis/Timing" % (run,tdc));
  f82.cd("/run%d/ChamberAll" % (run));
  c1=TCanvas();
  gStyle.SetOptFit();
  pos0=[]
  pmean0=[]
  pos1=[]
  pmean1=[]
  hall=f82.Get("/run%d/ChamberAll/XY" % (run));
  for i in range(sel,sel+1):
    pos0.append(0)
    pmean0.append(0)
    #hstrip=f82.Get("/run%d/TDC%d/LmAnalysis/Timing/hdtpos%d" % (run,tdc,i+71));
    hstrip=hall.ProjectionY("strip%d" % (i),(i),i+1)
    if (hstrip.GetEntries()<100):
      continue
    scfit=TF1("scfit","gaus",hstrip.GetMean()-2.*hstrip.GetRMS(),hstrip.GetMean()+2.*hstrip.GetRMS())
    hstrip.Fit("scfit","Q","",hstrip.GetMean()-2.*hstrip.GetRMS(),hstrip.GetMean()+2.*hstrip.GetRMS())
    dtmean=scfit.GetParameter(1)
    dtres=scfit.GetParameter(2)
    print(run,i,dtmean,dtres,dtmean*80./8.3,dtres*80/8.3)
    c1.Update()
    #val = input()
    time.sleep(2)


def fitdif(run):
  f82=TFile("./histo%d_0.root" % run);
  #f82.cd("/run%d/TDC%d/LmAnalysis/Timing" % (run,tdc));
  f82.cd("/run%d/ChamberDif" % (run));
  c1=TCanvas();
  gStyle.SetOptFit();
  pos0=[]
  pmean0=[]
  pos1=[]
  pmean1=[]
  for i in range(48):
    pos0.append(0)
    pmean0.append(0)
    #hstrip=f82.Get("/run%d/TDC%d/LmAnalysis/Timing/hdtpos%d" % (run,tdc,i+71));
    hstrip=f82.Get("/run%d/Timing/OneStrip/Side0/hdtr_%d" % (run,i+71));
    if (hstrip == None):
      continue
    if (hstrip.GetEntries()<20):
      continue
    hstrip.Rebin(2)
    
    hstrip.Draw()
    c1.Update()

    print("Enter min max")
    #hmin = float(input())
    #hmax = float(input())
    hmin=hstrip.GetMean()-5.*hstrip.GetRMS()
    hmax=hstrip.GetMean()+5.*hstrip.GetRMS()
    print(hmin,hmax)
    #scfit=TF1("scfit","gaus",hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    #hstrip.GetXaxis().SetRangeUser(hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    
    scfit=TF1("scfit","gaus",hmin,hmax)
    hstrip.GetXaxis().SetRangeUser(hmin,hmax)
    hstrip.Fit("scfit","Q");
    dtmean=scfit.GetParameter(1)
    dtres=scfit.GetParameter(2)
    hstrip.Draw()
    c1.Update()
    #c1.SaveAs("Run%d_Strip_pos.png" % (run));

    val = input()
    pmean0[i]=hstrip.GetMean()
    if (dtres<hstrip.GetRMS()):
      pos0[i]=dtmean
    else:
      pos0[i]=hstrip.GetMean()
      
  print(pos0)
  print(pmean0)
  for i in range(48):
    pos1.append(0)
    pmean1.append(0)
    #hstrip=f82.Get("/run%d/TDC%d/LmAnalysis/Timing/hdtpos%d" % (run,tdc,i+71));
    hstrip=f82.Get("/run%d/Timing/OneStrip/Side1/hdtr_%d" % (run,i+71));
    if (hstrip == None):
      continue
    if (hstrip.GetEntries()<20):
      continue
    hstrip.Rebin(2)
    
    hstrip.Draw()
    c1.Update()

    print("Enter min max")
    #hmin = float(input())
    #hmax = float(input())
    hmin=hstrip.GetMean()-5.*hstrip.GetRMS()
    hmax=hstrip.GetMean()+5.*hstrip.GetRMS()
    print(hmin,hmax)
    #scfit=TF1("scfit","gaus",hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    #hstrip.GetXaxis().SetRangeUser(hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    
    scfit=TF1("scfit","gaus",hmin,hmax)
    hstrip.GetXaxis().SetRangeUser(hmin,hmax)
    hstrip.Fit("scfit","Q");
    dtmean=scfit.GetParameter(1)
    dtres=scfit.GetParameter(2)
    hstrip.Draw()
    c1.Update()
    #c1.SaveAs("Run%d_Strip_pos.png" % (run));

    val = input()
    pmean1[i]=hstrip.GetMean()
    if (dtres<hstrip.GetRMS()):
      pos1[i]=dtmean
    else:
      pos1[i]=hstrip.GetMean()
      
  print(pos0)
  print(pmean0)





  
  dt=0
  for i in range(48):
    print("fe1_2tr[%d]=%5.3f;" % (i+71,pos0[i]-pos1[i]))

  #print(pos)

def fitpos(run,tdc):
  f82=TFile("./histo%d_0.root" % run);
  #f82.cd("/run%d/TDC%d/LmAnalysis/Timing" % (run,tdc));
  f82.cd("/run%d/Chamber%d/Timing" % (run,tdc));
  c1=TCanvas();
  gStyle.SetOptFit();
  pos=[]
  pmean=[]
  for i in range(48):
    pos.append(0)
    pmean.append(0)
    #hstrip=f82.Get("/run%d/TDC%d/LmAnalysis/Timing/hdtpos%d" % (run,tdc,i+71));
    hstrip=f82.Get("/run%d/Chamber%d/Timing/All/hdtpos%d" % (run,tdc,i+71));
    if (hstrip == None):
      continue
    hstrip.Rebin(2)
    
    hstrip.Draw()
    c1.Update()

    print("Enter min max")
    #hmin = float(input())
    #hmax = float(input())
    hmin=-20.
    hmax=20.
    print(hmin,hmax)
    #scfit=TF1("scfit","gaus",hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    #hstrip.GetXaxis().SetRangeUser(hstrip.GetMean()-3.*hstrip.GetRMS(),hstrip.GetMean()+3.*hstrip.GetRMS())
    
    scfit=TF1("scfit","gaus",hmin,hmax)
    hstrip.GetXaxis().SetRangeUser(hmin,hmax)
    hstrip.Fit("scfit","Q");
    dtmean=scfit.GetParameter(1)
    dtres=scfit.GetParameter(2)
    hstrip.Draw()
    c1.Update()
    #c1.SaveAs("Run%d_Strip_pos.png" % (run));

    val = input()
    pmean[i]=hstrip.GetMean()
    if (dtres>1. and  dtres<4):
      pos[i]=dtmean
    else:
      pos[i]=hstrip.GetMean()
  print(pos)
  print(pmean)
  
  for i in range(12):
    print("fe1_2tr[%d]=%5.3f;" % (i,pos[i]))
  #print(pos)

def calcefn(run,chamber,hv=0,dirp="."):
  f82=TFile("%s/histo%d_0.root" % (dirp,run));
  f82.cd("/run%d/Chamber%d/" % (run,chamber));
  c1=TCanvas();
  gStyle.SetOptFit();

  hns=f82.Get("/run%d/Chamber%d/Efficiency" % (run,chamber));
  #hns2=f82.Get("/run%d/TDC%d/LmAnalysis/NStrips2" % (run,tdc));
  hstrip=f82.Get("/run%d/Chamber%d/NSTRIP" % (run,chamber));
  hrate=f82.Get("/run%d/Chamber%d/Rate" % (run,chamber));

  hclusters=f82.Get("/run%d/Chamber%d/ClusterNew/Clusters" % (run,chamber));
  hfr=f82.Get("/run%d/Chamber%d/FebCount" % (run,chamber));
  hfrs=f82.Get("/run%d/Chamber%d/FebCountSel" % (run,chamber));
  hclusterm=f82.Get("/run%d/Chamber%d/ClusterNew/ClusterSize1" % (run,chamber));
  #hstrip.Rebin(2)nCont(I)
  febrate=0
  febs=(0,6500./4,7500./4)
  #print(hfr)
  febratesel=0
  if (hfr!=None):
    nevt_fr=hfr.GetBinContent(25);
    nmax=0
    nfb=0
    for i in range(0,24):
      #print(i,hfr.GetBinContent(i),nevt_fr)
      if (hfr.GetBinContent(i)>0):
        nmax=nmax+hfr.GetBinContent(i)
        nfb=nfb+1
    #print(nevt_fr,nfb,nmax)
    if (nevt_fr>0 and nfb>0):
      febrate=nmax*1./nfb/(nevt_fr*20E-9)
  if (hfrs!=None):
    nevt_frs=hfrs.GetBinContent(25);
    nmaxs=0
    nfbs=0
    for i in range(0,24):
      #print(i,hfr.GetBinContent(i),nevt_fr)
      if (hfrs.GetBinContent(i)>0):
        nmaxs=nmaxs+hfrs.GetBinContent(i)
        nfbs=nfbs+1
    #print(nevt_frs,nfbs,nmaxs)
    if (nevt_frs>0 and nfbs>0):
      febratesel=nmaxs*1./nfbs/(nevt_frs*20E-9)


  csize=0.1
  effc=0.0
  deffc=0.0
  ncev=0
  ncl=0
  if (hclusterm!=None):
    csize=hclusterm.GetMean()
    ncev=hclusters.GetEntries()
    nc=ncev-hclusters.GetBinContent(1)
    ncl=0
    nevcl=0
    for i in range(2,16):
        x=i-1.;
        y=hclusters.GetBinContent(i)
        nevcl=nevcl+y
        ncl=ncl+x*y
    if (nevcl>0):
        ncl=ncl*1./nevcl
    effc=nc*1./ncev
    deffc=math.sqrt(effc*(1-effc)/ncev)
  ntrg=hns.GetBinContent(3)
  nall=hns.GetBinContent(4)
  nxy=hns.GetBinContent(5)

  hstrip.GetXaxis().SetRangeUser(0.5,9.5)
  hrate.GetXaxis().SetRangeUser(0.1,60000.)
  mul=hstrip.GetMean()

  
  #print(ntrg,hns.GetBinContent(1),hns2.GetBinContent(1))
  eff=nall*1./ntrg
  deff=math.sqrt(eff*(1-eff)/ntrg)
  effp=nxy*1./ntrg
  deffp=math.sqrt(effp*(1-effp)/ntrg)
  
  print("|%d|%d|%7.1f|%d|%d|%d|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%7.1f|%d|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%5.2f|%5.1f" % (run,chamber,hv,int(ntrg),int(nall),int(nxy),eff*100,deff*100,effp*100,deffp*100,mul,hrate.GetMean(),ncev,effc*100,deffc*100,csize,ncl,febrate/febs[chamber],-febrate*10E-7,febratesel/febs[chamber]))
  #hstrip.Draw()
  #c1.Update()
  #c1.SaveAs("Run%d_Strip_pos.png" % (run));

  #val = input()
  r=(run,chamber,hv,int(ntrg),int(nall),int(nxy),eff*100,deff*100,effp*100,deffp*100,mul,hrate.GetMean(),ncev,effc*100,deffc*100,csize,ncl,febrate/febs[chamber],-febrate*10E-7,febratesel/febs[chamber])
  #r = map(prettyfloat, r)
  return r


def extractEfficiency(run,chamber,hv=0,dirp="."):
  f82=TFile("%s/histo%d_0.root" % (dirp,run));
  f82.cd("/run%d/InTime/Chamber%d/" % (run,chamber));
  #c1=TCanvas();
  #gStyle.SetOptFit();

  hclusters=f82.Get("/run%d/InTime/Chamber%d/ClusterNew/Clusters" % (run,chamber));
  hclustero=f82.Get("/run%d/OffTime/Chamber%d/ClusterNew/Clusters" % (run,chamber));

  hfr=f82.Get("/run%d/InTime/Chamber%d/FebCount" % (run,chamber));
  hfrs=f82.Get("/run%d/OffTime/Chamber%d/FebCountSel" % (run,chamber));
  hclusterm=f82.Get("/run%d/InTime/Chamber%d/ClusterNew/ClusterSize1" % (run,chamber));
  #hstrip.Rebin(2)nCont(I)
  febrate=0
  febs=(0,6500./4,7500./4)
  #print(hfr)
  febratesel=0
  if (hfr!=None):
    nevt_fr=hfr.GetBinContent(25);
    nmax=0
    nfb=0
    for i in range(0,24):
      #print(i,hfr.GetBinContent(i),nevt_fr)
      if (hfr.GetBinContent(i)>0):
        nmax=nmax+hfr.GetBinContent(i)
        nfb=nfb+1
    #print(nevt_fr,nfb,nmax)
    if (nevt_fr>0 and nfb>0):
      febrate=nmax*1./nfb/(nevt_fr*20E-9)
  if (hfrs!=None):
    nevt_frs=hfrs.GetBinContent(25);
    nmaxs=0
    nfbs=0
    for i in range(0,24):
      #print(i,hfr.GetBinContent(i),nevt_fr)
      if (hfrs.GetBinContent(i)>0):
        nmaxs=nmaxs+hfrs.GetBinContent(i)
        nfbs=nfbs+1
    #print(nevt_frs,nfbs,nmaxs)
    if (nevt_frs>0 and nfbs>0):
      febratesel=nmaxs*1./nfbs/(nevt_frs*20E-9)


  csize=0.1
  effc=0.0
  deffc=0.0
  effco=0.0
  deffco=0.0
  ncevo=0
  ncev=0
  ncl=0
  if (hclusterm!=None):
    csize=hclusterm.GetMean()
    ncev=hclusters.GetEntries()
    nc=ncev-hclusters.GetBinContent(1)
    ncl=0
    nevcl=0
    for i in range(2,32):
        x=i-1.;
        y=hclusters.GetBinContent(i)
        nevcl=nevcl+y
        ncl=ncl+x*y
    if (nevcl>0):
        ncl=ncl*1./nevcl
    effc=nc*1./ncev
    deffc=math.sqrt(effc*(1-effc)/ncev)
    ncevo=hclustero.GetEntries()
    nco=ncevo-hclustero.GetBinContent(1)
    effco=nco*1./ncevo
    if (ncevo>0 and effco>0):
        #print(ncevo,effco)
        deffco=math.sqrt(effco*(1-effco)/ncevo)

  
  print("|%d|%d|%7.1f|%d|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%5.2f|%5.1f|%5.2f|%5.2f|" % (run,chamber,hv,ncev,effc*100,deffc*100,csize,ncl,febrate/febs[chamber],-febrate*10E-7,febratesel/febs[chamber],effco*100,100*(effc-effco)/(1.-effco)))
  #hstrip.Draw()
  #c1.Update()
  #c1.SaveAs("Run%d_Strip_pos.png" % (run));

  #val = input()
  r=(run,chamber,hv,ncev,effc*100,deffc*100,csize,ncl,febrate/febs[chamber],-febrate*10E-7,febratesel/febs[chamber],effco*100,100*(effc-effco)/(1.-effco))
  #r = map(prettyfloat, r)
  return r



def calceff(run,tdc,strip=71):
  f82=TFile("./histo%d_0.root" % run);
  f82.cd("/run%d/TDC%d/LmAnalysis" % (run,tdc));
  c1=TCanvas();
  gStyle.SetOptFit();

  heff=f82.Get("/run%d/TDC%d/LmAnalysis/Efficiency" % (run,tdc));
  hstrip=f82.Get("/run%d/TDC%d/LmAnalysis/hdts%d" % (run,tdc,strip));
  hstrip.Rebin(2)
  scfit=TF1("scfit","gaus",hstrip.GetMean()-6.*hstrip.GetRMS(),hstrip.GetMean()+6.*hstrip.GetRMS())
  hstrip.GetXaxis().SetRangeUser(hstrip.GetMean()-6.*hstrip.GetRMS(),hstrip.GetMean()+6.*hstrip.GetRMS())
  hstrip.Fit("scfit","Q");
  dtmean=scfit.GetParameter(1)
  dtres=scfit.GetParameter(2)

  ntrg=heff.GetBinContent(2)
  n1=heff.GetBinContent(3)
  n2=heff.GetBinContent(4)
  eff=n1/ntrg
  deff=math.sqrt(eff*(1-eff)/ntrg)
  effp=n2/ntrg
  deffp=math.sqrt(effp*(1-effp)/ntrg)
  
  print("|%d|150|%d|%d|%d|%5.2f|%5.2f|%5.2f|%5.2f|%5.2f|%5.3f|" % (run,int(ntrg),int(n1),int(n2),eff*100,deff*100,effp*100,deffp*100,dtmean,dtres))
  hstrip.Draw()
  c1.Update()
  c1.SaveAs("Run%d_Strip%d_pos.png" % (run,strip));

  val = input()

def fitped(run,tdc,vthmin,vthmax,asic=1,ncha=24,rising=True,old=defped):
  rb=1
  fi=1
  la=ncha+1
  #if (asic==2):
  #    fi=ncha+1
  #    la=2*ncha+1
  asicmap={}
  for i in {1,2}:
      asicmap[i]=[]
      for j in range(49):
          asicmap[i].append(0)

  asicmap[1][1]=30
  asicmap[1][2]=28
  asicmap[1][3]=26
  asicmap[1][4]=24
  asicmap[1][5]=23
  asicmap[1][6]=22
  asicmap[1][7]=21
  asicmap[1][8]=20
  asicmap[1][9]=19
  asicmap[1][10]=18
  asicmap[1][11]=17
  asicmap[1][12]=16
  asicmap[1][13]=15
  asicmap[1][14]=14
  asicmap[1][15]=13
  asicmap[1][16]=12
  asicmap[1][17]=11
  asicmap[1][18]=10
  asicmap[1][19]=9
  asicmap[1][20]=8
  asicmap[1][21]=7
  asicmap[1][22]=6
  asicmap[1][23]=5
  asicmap[1][24]=4
  asicmap[2][25]=30
  asicmap[2][26]=28
  asicmap[2][27]=26
  asicmap[2][28]=24
  asicmap[2][29]=23
  asicmap[2][30]=22
  asicmap[2][31]=21
  asicmap[2][32]=20
  asicmap[2][33]=19
  asicmap[2][34]=18
  asicmap[2][35]=17
  asicmap[2][36]=16
  asicmap[2][37]=15
  asicmap[2][38]=14
  asicmap[2][39]=13
  asicmap[2][40]=12
  asicmap[2][41]=11
  asicmap[2][42]=10
  asicmap[2][43]=9
  asicmap[2][44]=8
  asicmap[2][45]=7
  asicmap[2][46]=6
  asicmap[2][47]=5
  asicmap[2][48]=4


  asicmap={}
  for i in {1,2}:
    asicmap[i]=[]
    for j in range(50):
      asicmap[i].append(0)
  f=open("/opt/TdcAnalysis/feb_mapping.json")
  s=json.loads(f.read())
  prh=s["v1_56"]["FlexTop"]["High"]["PR"]
  prl=s["v1_56"]["FlexTop"]["Low"]["PR"]
  tdch0=s["v1_56"]["FlexTop"]["High"]["TDC"][0]
  tdch1=s["v1_56"]["FlexTop"]["High"]["TDC"][1]
  tdcl0=s["v1_56"]["FlexTop"]["Low"]["TDC"][0]
  tdcl1=s["v1_56"]["FlexTop"]["Low"]["TDC"][1]
  for i in range(12):
      asicmap[1][tdch0[i]]=prh[i]
      asicmap[1][tdcl0[i]]=prl[i]
      asicmap[2][tdcl1[i]]=prl[i]
      asicmap[2][tdch1[i]]=prh[i]
  print(asicmap)
  ped=[]
  for i in range(32):
    ped.append(0)
  f82=TFile("Histos/InTime/histo%d_0.root" % run);
  f82.cd("/run%d/TDC%d" % (run,tdc));
  c1=TCanvas();
  #c2=TCanvas("c2","Test",1400,900);
  #c2.cd()
  #c2.Divide(6,4)
  #c2.Draw()
  #c2.Update()
  #val = input()
  #c2.Draw()
  fout=open("summary_pedestal_%d_tdc%d.txt" % (run,tdc),"w");
  fout.write("+--+-----+-----+-----+ \n");
  gStyle.SetOptFit();
  hmean=TH1F("hmean","Summary %d %d " %(run,tdc),vthmax-vthmin+1,vthmin,vthmax)
  hnoise=TH1F("hnoise","Summary noise %d %d " %(run,tdc),100,0.,30.)
  hpmean=TH1F("hpmean","Summary %d %d " %(run,tdc),2*ncha,0.,2.*ncha);
  hpnoise=TH1F("hpnoise","Summary noise %d %d " %(run,tdc),2*ncha,0.,2.*ncha);
  scfit=TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",vthmin+1,vthmax);
  
  for ip in range(la-1,fi,-1):
      #c2.cd()
      if (asicmap[asic][ip]==0):
          continue;
      hs=None
      if (rising):
          hs=f82.Get("/run%d/TDC%d/vthc%d" % (run,tdc,ip));
      else:
          hs=f82.Get("/run%d/TDC%d/vthd%d" % (run,tdc,ip));
      if (hs==None):
          continue;
      if (hs.GetEntries()==0):
        continue
      print(ip,fi,la," found")
      hs.Scale(1./2700.);
      nmax=0
      for i in range(1,hs.GetNbinsX()):
        if (hs.GetBinContent(i)==0):
              if (hs.GetBinContent(i-1)!=0 and hs.GetBinContent(i+1)!=0):
                  hs.SetBinContent(i,(hs.GetBinContent(i-1)+hs.GetBinContent(i+1))/2.)
        else:
          if (hs.GetBinContent(i)>nmax):
              nmax=hs.GetBinContent(i)

      hs.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      icolor= ip%4 +1
      istyle= ip/4+1
      hs.SetLineColor(icolor)
      hs.SetLineStyle(int(istyle))
      hs.SetLineWidth(2)
      c1.cd()
      c1.Draw()
      



      if (ip==0):
        hs.Draw()
      else:
        hs.Draw("SAME")
  c1.Update()
  c1.SaveAs("Run%d_AllStrip%d_%d.root" % (run,tdc,asic));
  c1.SaveAs("Run%d_AllStrip%d_%d.png" % (run,tdc,asic));

  val = input()
      
  for ip in range(fi,la):
      #c2.cd()
      if (asicmap[asic][ip]==0):
          continue;

      hs=None
      if (rising):
          hs=f82.Get("/run%d/TDC%d/vthc%d" % (run,tdc,ip));
      else:
          hs=f82.Get("/run%d/TDC%d/vthd%d" % (run,tdc,ip));
      if (hs==None):
          continue;
      if (hs.GetEntries()==0):
        continue
      #hs.Scale(1./2700.);
      hder=TH1F("hder%d" % ip,"derivative",900/rb,0.,900.)	
      hs.Rebin(rb)
      nmax=0
      for i in range(1,hs.GetNbinsX()):
          if (hs.GetBinContent(i)==0):
              if (hs.GetBinContent(i-1)!=0 and hs.GetBinContent(i+1)!=0):
                  hs.SetBinContent(i,(hs.GetBinContent(i-1)+hs.GetBinContent(i+1))/2.)
          else:
            if (hs.GetBinContent(i)>nmax):
              nmax=hs.GetBinContent(i)
      for i in range(1,hs.GetNbinsX()):
        if (hs.GetBinContent(i)-hs.GetBinContent(i+1)>-10):
          hder.SetBinContent(i,hs.GetBinContent(i)-hs.GetBinContent(i+1))
      hder.Rebin(2)
      hder.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      scfit.SetParameter(0,nmax/2.);
      scfit.SetParameter(1,hder.GetMean());
      scfit.SetParameter(2,hder.GetRMS());



      hs.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      hs.Fit("scfit","Q","",vthmin+2,vthmax);
      #hs.GetXaxis().SetRangeUser(vthmin-1,scfit.GetParameter(1)+60);
      #gPad.SetLogy();
      rped=scfit.GetParameter(1)
      c1.cd()
      c1.Draw()
      

      hder.Draw()
      c1.Update()
      val1 = input()

      print("heho ",val1,rped,hder.GetMean())
      rped=hder.GetMean()
      if (len(val1)>0):
          rped=float(val1)
      hs.Draw()
      

      c1.cd()
      c1.Draw()
      c1.Update()

      fout.write("|%2d|%5.1f|%5.1f|%5.2f| \n" % (ip,scfit.GetParameter(0),rped,scfit.GetParameter(2)));
      ipr=0
      if (ip%2==1):
        ipr=ip/2
      else:
        ipr=31-ip/2
      firmwaret=[31,29,27,25,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6]
      #firmwaco=[31,29,27,25,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4]
      firmware2=[24,5,3,1,0,2,4,6,7,8,9,10,26,28,30,31,29,27,25,23,22,21,20,19]
      
      firmwareta1=[21,20,23,22,25,24,27,26,29,28,31,30,1,0,3,2,5,4,7,6,10,8,15,12]
      firmwareta2=[21,20,23,22,25,24,27,26,29,28,31,30,1,0,3,2,5,4,7,6,10,8,14,12]
      firmwaretb=[30,26,28,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,30,26,28,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4]

      if (asic==1):
          firmware=firmwareta1
      else:
          firmware=firmwareta2
      firmware=firmwaretb
      #if (ip>0):
      #  ipr=firmware[ip-fi-1]
      #else:
      #  ipr=0
      print(ip,fi,ip-fi)
      ipr=firmware[ip-fi]
      ipr=asicmap[asic][ip]
      ped[ipr]=rped
      print(ip,ipr,rped,scfit.GetParameter(2))
      hmean.Fill(rped)
      hnoise.Fill(scfit.GetParameter(2))
      hpmean.SetBinContent(ip+1,rped);
      hpnoise.SetBinContent(ip+1,scfit.GetParameter(2))
      #c1.SaveAs("Run%d_Strip%d.root" % (run,ip));
      val = input()

      #hder.Draw()
      
      #c1.Update()
      #val = input()
  c1.cd()
  hmean.Draw()
  hpmean.GetYaxis().SetRangeUser(vthmin,vthmax)
  hpmean.Draw()
  c1.Update()
  c1.SaveAs("Summary_%d_TDC%d.png" % (run,tdc));
  val = input()
  hnoise.Draw()
  c1.Update()
  val = input()
  hpnoise.Draw()
  c1.Update()
  val = input()
  c1.Update()
  c1.SaveAs("Summary_Noise_%d_TDC%d.png" % (run,tdc));

  fout.write("+--+-----+-----+-----+ \n");
  fout.close()
  print(ped)
  val = input()
  med=5550.0
  for i in range(32):
    if (ped[i]==0):
      continue;
    if (ped[i] < med):
      #print(med,ped[i])
      med=ped[i]

  med=med+5
  med=480
  print("Alignment to :",med)
  dac=ped
  for i in range(32):
    if (ped[i]==0):
      continue;
    old[i]=0
    dac[i]=int(round(old[i]+(med-ped[i])*1./2.97))
  print("cor%d_%d=" % (tdc,asic),dac)
  return dac

def calcped(oldpr,ped,median):
  print("Alignment to :",median)
  dac=[]
  for i in range(32):
    dac.append(0)

  for i in range(32):
    if (ped[i]<=0):
      continue;
    dac[i]=int(round(oldpr[i]+(median-ped[i])*1./2.97))
  print(dac)
  return dac
import os
def process(runs,proc=True):
  if (proc):  
    for run in runs:
      os.system("./bin/tdcr -ggifpp_geom.json -r%d " % run)

  v=6800
  for run in runs:
    calcefn(run,1,v)
    v=v+200

  v=6800
  for run in runs:
    calcefn(run,2,v)
    v=v+200

def proclist(first,last,proc=True,vf=6700,step=100):
  if (proc):  
    for run in range(first,last+1):
      os.system("./bin/tdcr -ggifpp_geom.json -r%d " % run)

  v=vf
  for run in range(first,last+1):
    calcefn(run,1,v)
    v=v+step

  v=vf
  for run in range(first,last+1):
    calcefn(run,2,v)
    v=v+step
def processDCS(fdb,webdcs,proc=True,diro="."):
    conn = sqlite3.connect(fdb)
    conn.text_factory = str
    curs = conn.cursor()
    curs.execute("SELECT RUN,HV FROM runs WHERE DCS=%d" % webdcs)
    v=curs.fetchall()
    #print(v[0])
    #print(v[0][0])
    #return
    if (proc):  
        for x in v:
            os.system("./bin/tdcr -ggifpp_geom.json -r%d " % x[0])
            
    res=[]        
    for x in v:
        res.append(extractEfficiency(x[0],1,x[1],dirp=diro))
    for x in v:
        res.append(extractEfficiency(x[0],2,x[1],dirp=diro))
    return res

def storeResults(fdbi,fdbo,webdcs,histod=".",runtype=1):
    conn = sqlite3.connect(fdbo)
    conn.text_factory = str

    sql_create_res_table = """CREATE TABLE IF NOT EXISTS corana (
                                    ID integer PRIMARY KEY,
                                    RUN INTEGER NOT NULL,
                                    CHAMBER INTEGER NOT NULL,
                                    TYPE INTEGER NOT NULL,
                                    HV REAL,
                                    NCEVT REAL,
                                    EFFC REAL,
                                    DEFFC REAL,
                                    CSIZE REAL,
                                    NCLUS REAL,
                                    DAQFEBRATE REAL,
                                    DAQEFFLOSS REAL,
                                    XYFEBRATE REAL,
                                    EFFBACK REAL,
                                    EFFCOR  REAL

                                );"""

#febrate/febs[chamber],-febrate*10E-7,febratesel/febs[chamber])
    try:
        c = conn.cursor()
        c.execute(sql_create_res_table)
        conn.commit()
    except Error as e:
        print(e)






    
    curs = conn.cursor()
    res=processDCS(fdbi,webdcs,False,diro=histod)
    #run,chamber,hv,int(ntrg),int(nall),int(nxy),eff*100,deff*100,effp*100,deffp*100,mul,hrate.GetMean(),ncev,effc*100,deffc*100,csize,ncl]
    for r in res:
        #print(len(r))
        sql_ins='''INSERT INTO corana(run,chamber,type,hv,ncevt,effc,deffc,csize,nclus,daqfebrate,daqeffloss,xyfebrate,effback,effcor) VALUES(%d,%d,%d,%d,%5.1f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f)''' % (r[0],r[1],runtype,r[2],r[3],r[4],r[5],r[6],r[7],r[8],r[9],r[10],r[11],r[12])
        print(sql_ins)
        curs.execute(sql_ins)
        conn.commit()
    
def buildTGraph(title,vx,dvx,vy,dvy,tx,ty):
    r_ti, r_p = array( 'd' ), array( 'd' )
    er_ti, er_p = array( 'd' ), array( 'd' )
    for x in vx:
      r_ti.append(x)
    for y in vy:
      r_p.append(y)
    for x in dvx:
      er_ti.append(x)
    for y in dvy:
      er_p.append(y)

    gr = TGraphErrors( len(vx), r_ti, r_p,er_ti,er_p )
    gr.SetLineColor( 1 )
    gr.SetLineWidth( 1 )
    gr.SetMarkerColor( 2 )
    gr.SetMarkerStyle( 21 )
    gr.SetMarkerSize( 0.4 )
    gr.SetTitle(title)
    gr.GetXaxis().SetTitle(tx )
    gr.GetYaxis().SetTitle( ty )
    return gr

def buildTGraph1(title,vx,vy,tx,ty):
    r_ti, r_p = array( 'd' ), array( 'd' )
    er_ti, er_p = array( 'd' ), array( 'd' )
    for x in vx:
      r_ti.append(x)
    for y in vy:
      r_p.append(y)
   
    
    gr = TGraph( len(vx), r_ti, r_p)
    gr.SetLineColor( 1 )
    gr.SetLineWidth( 1 )
    gr.SetMarkerColor( 2 )
    gr.SetMarkerStyle( 21 )
    gr.SetMarkerSize( 0.4 )
    gr.SetTitle(title)
    gr.GetXaxis().SetTitle(tx )
    gr.GetYaxis().SetTitle( ty )
    return gr
def  drawDCS(fdbi,webdcs,chamber,c=None):
    if (c==None):
        c=TCanvas()
    jdict={}
    jdict['settings']={}
    gStyle.SetOptFit(1)
    c.Clear()
    #c.Divide(2,4)
    conn = sqlite3.connect(fdbi)
    conn.text_factory = str
    curs = conn.cursor()
    sql_dcs="select ATT,DEAD,TRET,TCOAX,TRIGGER,ACTIVE,START from webdcs WHERE dcs=%d" % webdcs
    curs.execute(sql_dcs)
    v=curs.fetchall()
    att=-1
    dthr=-1
    dead=-1
    trig="UNKNOwN"
    if (len(v)<1):
        return
    jdict['settings']['dcs']=webdcs
    jdict['settings']['att']=v[0][0]
    jdict['settings']['dead']=v[0][1]
    jdict['settings']['tret']=v[0][2]
    jdict['settings']['tcoax']=v[0][3]
    jdict['settings']['trigger']=v[0][4]
    jdict['settings']['active']=v[0][5]
    jdict['settings']['start']=v[0][6]

    dirout="./results/dcs/%d_ATT%3.1f_DT%d_THR%d_%s/chamber%d/" % (webdcs,v[0][0],v[0][1],v[0][2]-500,v[0][4].replace('"','').replace(' ','_'),chamber)
    os.system("mkdir -p %s" % dirout)
    fout=open(dirout+"summary%d_ATT%3.1f_DT%d_THR%d_%s.txt" % (webdcs,v[0][0],v[0][1],v[0][2]-500,v[0][4].replace('"','').replace(' ','_')),"w")
    for x in v:
        fout.write("Cuts: %5.2f %d %d %d %s %d %s\n" % x)
        att=x[0]
        dead=x[1]
        dthr=x[2]-500
        trig=x[4]
    sql_query=" select EFFCOR,DEFFC,(SELECT HV FROM runs WHERE runs.RUN=corana.RUN),DAQFEBRATE,DAQEFFLOSS,RUN,NCEVT,EFFC,CSIZE,NCLUS,XYFEBRATE,EFFBACK  from corana WHERE RUN IN (SELECT RUN FROM runs WHERE DCS=%d) AND CHAMBER=%d" % (webdcs,chamber)
    curs.execute(sql_query)
    vo=curs.fetchall()
    if (len(vo)<1):
        return;
    hv=[]
    eff=[]
    dhv=[]
    deff=[]
    febrate=[]
    dfebrate=[]
    effloss=[]
    csize=[]
    nclus=[]
    effback=[]
    jdict['runs']={}
    jdict['runs']['effcor']=[]
    jdict['runs']['hv']=[]
    jdict['runs']['febrate']=[]
    jdict['runs']['effloss']=[]
    jdict['runs']['runid']=[]
    jdict['runs']['nevt']=[]
    jdict['runs']['effclu']=[]
    jdict['runs']['csize']=[]
    jdict['runs']['nclus']=[]
    jdict['runs']['xyrate']=[]
    jdict['runs']['effback']=[]

    
    for x in vo:
        if (x[1]==0):
            continue
        fout.write("Results: %5.2f %5.2f %5.2f %5.2f %5.2f %d %d %5.2f %5.2f %5.2f %5.2f %5.2f \n" % x)
        hv.append(x[2])
        dhv.append(10.)
        eff.append(x[0])
        deff.append(x[1])
        febrate.append(x[3])
        effloss.append(x[4])
        csize.append(x[8])
        nclus.append(x[9])
        effback.append(x[11])
        jdict['runs']['effcor'].append(x[0])
        jdict['runs']['hv'].append(x[2])
        jdict['runs']['febrate'].append(x[3])
        jdict['runs']['effloss'].append(x[4])
        jdict['runs']['runid'].append(x[5])
        jdict['runs']['nevt'].append(x[6])
        jdict['runs']['effclu'].append(x[7])
        jdict['runs']['csize'].append(x[8])
        jdict['runs']['nclus'].append(x[9])
        jdict['runs']['xyrate'].append(x[10])
        jdict['runs']['effback'].append(x[11])
    stitle="DCS%d_TRG%s_ATT%3.1f_THR%d_DT%d_CH%d" % (webdcs,trig,att,dthr,dead,chamber)
    gr=buildTGraph("effi",hv,dhv,eff,deff,"HV effective (V)","efficiency (%)")

    func = TF1("func", "([0]/(1+ TMath::Exp(-[1]*(x-[2]))))", 6500,8200)
    func.SetParameters(90, 9.E-3, 7000)
    print(100, 9.E-3, 7100)
    gr.Fit(func,"","",6700,8200)
    hv95=func.GetX(func.GetParameter(0)*0.95)
    hv99=func.GetX(func.GetParameter(0)*0.99)
    print("HV95",hv95,hv95+150,hv99,hv99-hv95)
    wp=hv99
    jdict['fit']={}
    jdict['fit']['Efficiency']=func.GetParameter(0)
    jdict['fit']['Slope']=func.GetParameter(1)
    jdict['fit']['HV50']=func.GetParameter(2)
    jdict['fit']['HV95']=hv95
    jdict['fit']['HV99']=wp

    fout.write("FIT results: %f %f %f %f %f \n" % (func.GetParameter(0),func.GetParameter(1),func.GetParameter(2),hv95,wp))
    title=stitle+"_HV95_%4.0f_WP_%4.0f" % (hv95,wp)
    gr.SetTitle(title)
    gStyle.SetStatX(0.85)
    gStyle.SetStatY(0.7)
    c.cd(1)
    gr.Draw("AP")
    c.Update()
    #val=input()
    c.SaveAs(dirout+"%s.png" % title)

    tgr=[]
    tgr.append(buildTGraph1('FEB Rate',hv,febrate,'HV eff (V)','Rate (Hz/cm^2)'))
    tgr.append(buildTGraph1('Dead Time Loss',hv,effloss,'HV eff (V)','Dead time loss (%)'))
    tgr.append(buildTGraph1('Cluster Size',hv,csize,'HV eff (V)','Cluster size'))
    tgr.append(buildTGraph1('Cluster Number',hv,nclus,'HV eff (V)','Clusters'))
    tgr.append(buildTGraph1('Background Efficiency',hv,effback,'HV eff (V)','Efficiency (%)'))
    #val=input()
    # grb=buildTGraph("background",hv,dhv,febrate,dfebrate,"HV effective (V)","FEB rate (Hz/cm^2)")
    # title=stitle
    # grb.SetTitle(title)
    # gStyle.SetStatX(0.85)
    # gStyle.SetStatY(0.7)
    # c.cd(2)
    # grb.Draw("AP")
    # c.Update()
    # c.SaveAs(dirout+"%s.png" % title)
    #val=input()
    sql_query="select * from RESULTS WHERE  CTIME>=(SELECT CFIRST FROM webdcs WHERE DCS=%d) AND CTIME<=(SELECT CLAST FROM webdcs WHERE DCS=%d)-200 AND HARDWARE='BMP'" % (webdcs,webdcs)
    curs.execute(sql_query)
    v=curs.fetchall()
    bmp=[]
    for x in v:
        bmp.append([x[3],json.loads(x[4].decode('latin-1').encode("utf-8"))])
    #print(bmp)
    PB=0
    TB=0
    NB=0
    for x in bmp:
      PB=PB+x[1]['pressure']
      TB=TB+x[1]['temperature']
      NB=NB+1
    if (NB>0):
      PB=PB/NB
      TB=TB/NB+273.15
    fout.write("Pressure: %d %5.2f %5.2f \n" % (NB,PB,TB))
    jdict['BMP']={"P":PB,"T":TB}

    #val=input()
    sql_query="select * from RESULTS WHERE  CTIME>=(SELECT CFIRST FROM webdcs WHERE DCS=%d) AND CTIME<=(SELECT CLAST FROM webdcs WHERE DCS=%d)-200 AND HARDWARE='SY1527'" % (webdcs,webdcs)
    curs.execute(sql_query)
    v=curs.fetchall()
    
    a=[]
    for x in v:
        a.append([x[3],json.loads(x[4].decode('latin-1').encode("utf-8"))])
    #print(a)
    #val=input()
    if (chamber==1):
      chan=[4,5]
    else:
      chan=[1,2] 
    idx=0
    #tgr=[]
    jdict['HV']={}
    for ch in chan:
          x_t=[]
          y_vs=[]
          z_vm=[]
          w_im=[]
          g_g=[]
          chname=""
          first=0
          for x in a:
            if (x[1]['channels'][ch]['rampup']==0):
              continue
            if (float(x[1]['channels'][ch]['vset']) < 5500.0):
              continue
            #print(float(x[1]['channels'][ch]['vset']))
            if (first==0):
              first=x[0]
            x_t.append(x[0]-first)
            y_vs.append(x[1]['channels'][ch]['vset'])
            z_vm.append(x[1]['channels'][ch]['vout'])
            w_im.append(x[1]['channels'][ch]['iout'])
            g_g.append( x[1]['channels'][ch]['iout']/ x[1]['channels'][ch]['vout'])
            #print(ch,x)
            chname=x[1]['channels'][ch]['name']
          if (len(x_t)<1):
            continue
          dy_vs=[]
          if (len(y_vs)>1):
            for i in range(0,len(y_vs)):
              if (i==0 and i!=len(y_vs)-1):
                dy_vs.append(y_vs[i+1]-y_vs[i])
                continue
              if (i==len(y_vs)-1 and i!=0):
                dy_vs.append(y_vs[i]-y_vs[i-1])
                continue
              dy_vs.append((y_vs[i+1]-y_vs[i-1])/2.)
          else:
            dy_vs.append(0)
          vset=[]
          vmon=[]
          veff=[]
          imon=[]
          nval=0
          vmon_sum=0
          vset_sum=0
          imon_sum=0
          for i in range(0,len(y_vs)):
            if (abs(dy_vs[i])>10):
              if (nval>0):
                vmon.append(vmon_sum/nval)
                vset.append(vset_sum/nval)
                imon.append(imon_sum/nval)
                if (NB>0):
                  veff.append(calV(vmon_sum/nval,PB,TB))
                else:
                  veff.append(vmon_sum/nval)
              nval=0
              vmon_sum=0
              vset_sum=0
              imon_sum=0
              continue
            nval=nval+1
            vmon_sum=vmon_sum+z_vm[i]
            vset_sum=vset_sum+y_vs[i]
            imon_sum=imon_sum+w_im[i]
          if (nval>0):
            vmon.append(vmon_sum/nval)
            vset.append(vset_sum/nval)
            imon.append(imon_sum/nval)
            if (NB>0):
              veff.append(calV(vmon_sum/nval,PB,TB))
            else:
              veff.append(vmon_sum/nval)
          #print(chname,"VSET",vset)
          #print(chname,"VMON",vmon)
          #print(chname,"VEFF",veff)
          #print(chname,"IMON",imon)
          fout.write(chname+"\n")
          jdict['HV'][chname]={}
          jdict['HV'][chname]['vset']=vset
          jdict['HV'][chname]['vmon']=vmon
          jdict['HV'][chname]['imon']=imon
          jdict['HV'][chname]['veff']=veff

          for ip in range(0,len(vmon)):
            fout.write("%5.2f %5.2f %5.2f %5.2f \n" % (vset[ip],vmon[ip],veff[ip],imon[ip]))
          #tgr.append(buildTGraph1('V set vs t  %s' % chname,x_t,y_vs,'t(s)','V set (V)'))
          tgr.append(buildTGraph1('V Mon vs t  %s' % chname,x_t,z_vm,'t(s)','V mon (V)'))
          tgr.append(buildTGraph1('I Mon vs t  %s' % chname,x_t,w_im,'t(s)','I mon ([m]A)'))
          tgr.append(buildTGraph1('I Mon vs V eff %s' % chname,veff,imon,'V eff(V)','I mon ([m]A)'))
    icd=3
    for x in tgr:
      c.cd(icd)
      icd=icd+1
      x.Draw("AP")
      c.Update()
      c.SaveAs(dirout+"%s.png" % x.GetTitle().replace(" ","_"))
      #val=input()      
    # resume
    chans=[]
    if (chamber == 2):
      chans=["COAX-BOT","COAX-TOP"]
    else:
      chans=["RETURN-BOT","RETURN-TOP"]
    jdict['AWP']={}
    jdict['AWP']['hv']=wp
    ihv=-1
    for i in range(0,len(jdict['runs']['hv'])-1):
      if (wp>jdict['runs']['hv'][i] and wp<=jdict['runs']['hv'][i+1]):
        ihv=i
        break
    if (ihv!=-1):
      jdict['AWP']['plateau']=jdict['fit']['Efficiency']
      jdict['AWP']['febrate']=approx(wp,jdict['runs']['hv'][ihv],jdict['runs']['hv'][ihv+1],jdict['runs']['febrate'][ihv],jdict['runs']['febrate'][ihv+1])
      jdict['AWP']['effloss']=approx(wp,jdict['runs']['hv'][ihv],jdict['runs']['hv'][ihv+1],jdict['runs']['effloss'][ihv],jdict['runs']['effloss'][ihv+1])
      jdict['AWP']['effcor']=approx(wp,jdict['runs']['hv'][ihv],jdict['runs']['hv'][ihv+1],jdict['runs']['effcor'][ihv],jdict['runs']['effcor'][ihv+1])
      jdict['AWP']['csize']=approx(wp,jdict['runs']['hv'][ihv],jdict['runs']['hv'][ihv+1],jdict['runs']['csize'][ihv],jdict['runs']['csize'][ihv+1])
      jdict['AWP']['nclus']=approx(wp,jdict['runs']['hv'][ihv],jdict['runs']['hv'][ihv+1],jdict['runs']['nclus'][ihv],jdict['runs']['nclus'][ihv+1])
    for x in chans:
      ihv=-1
      if (x in jdict['HV']):
        for i in range(0,len(jdict['HV'][x]['veff'])-1):
          if (wp>jdict['HV'][x]['veff'][i] and wp<=jdict['HV'][x]['veff'][i+1]):
            ihv=i
            break
      if (ihv!=-1):
         jdict['AWP'][x]=approx(wp,jdict['HV'][x]['veff'][ihv],jdict['HV'][x]['veff'][ihv+1],jdict['HV'][x]['imon'][ihv],jdict['HV'][x]['imon'][ihv+1])
    itot=0
    surf=0
    if (chamber==1):
      surf=13000
      if ('RETURN-TOP' in jdict['AWP']):
        itot=itot+jdict['AWP']['RETURN-TOP']
      if ('RETURN-BOT' in jdict['AWP']):
        itot=itot+jdict['AWP']['RETURN-BOT']
    if (chamber==2):
      surf=15000
      if ('COAX-TOP' in jdict['AWP']):
        itot=itot+jdict['AWP']['COAX-TOP']
      if ('COAX-BOT' in jdict['AWP']):
        itot=itot+jdict['AWP']['COAX-BOT']
    if ('febrate' in jdict['AWP']):
      if (jdict['AWP']['febrate']!=0):
        jdict['AWP']['ITOT']=itot
        jdict['AWP']['QSEEN']=itot/jdict['AWP']['febrate']/surf*1E6
        fout.write("%d|%5.1f|%5.0f|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%5.1f|%5.1f|%5.1f\n" % 
        (webdcs,
        jdict['settings']['att'],
        jdict['AWP']['hv'],
        jdict['AWP']['plateau'],
        jdict['AWP']['effcor'],
        jdict['AWP']['effloss'],
        jdict['AWP']['plateau']-jdict['AWP']['effloss'],
        jdict['AWP']['febrate'],
        jdict['AWP']['ITOT'],
        jdict['AWP']['QSEEN'],jdict['AWP']['csize']))
    with open(dirout+'summary.json', 'w') as outfile:
      dd= json.dumps(jdict,sort_keys=True, indent=2,separators=(',', ': '))
      outfile.write(dd)
      outfile.close()
    fout.close()
    return jdict


def processAllDCS(fdb,dbo,diro=".",proc=True,store=True,draw=False,canvas=None,first=0,last=100000):
    fout=open('resume.txt','w')
    conn = sqlite3.connect(fdb)
    conn.text_factory = str
    curs = conn.cursor()
    curs.execute("select distinct(dcs) from webdcs where dcs>=%d and dcs<=%d" % (first,last))
    v=curs.fetchall()
    for x in v:
        if (proc):
            processDCS(fdb,x[0],proc,diro)
        if (store):
          storeResults(fdb,dbo,x[0],diro)
        if (draw):
          jdict=drawDCS(fdb,x[0],1,canvas)
          if ('QSEEN' in jdict['AWP']):
            fout.write("%d|%d|%s|%5.1f|%5.1f|%5.0f|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%5.1f|%5.1f\n" % 
            (jdict['settings']['dcs'],1,
            jdict['settings']['trigger'],
            jdict['AWP']['febrate'],
            jdict['settings']['att'],
            jdict['AWP']['hv'],
            jdict['AWP']['plateau'],
            jdict['AWP']['effcor'],
            jdict['AWP']['effloss'],
            jdict['AWP']['plateau']-jdict['AWP']['effloss'],
            jdict['AWP']['ITOT'],
            jdict['AWP']['QSEEN'],jdict['AWP']['csize']))
          jdict=drawDCS(fdb,x[0],2,canvas)
          if ('QSEEN' in jdict['AWP']):
            fout.write("%d|%d|%s|%5.1f|%5.1f|%5.0f|%5.2f|%5.2f|%5.2f|%5.2f|%5.1f|%5.1f|%5.1f\n" % 
            (jdict['settings']['dcs'],2,
            jdict['settings']['trigger'],
            jdict['AWP']['febrate'],
            jdict['settings']['att'],
            jdict['AWP']['hv'],
            jdict['AWP']['plateau'],
            jdict['AWP']['effcor'],
            jdict['AWP']['effloss'],
            jdict['AWP']['plateau']-jdict['AWP']['effloss'],
            jdict['AWP']['ITOT'],
            jdict['AWP']['QSEEN'],jdict['AWP']['csize']))
    fout.close()
def approx(v,v1,v2,a1,a2):
  return a1+(v-v1)*(a2-a1)*1./(v2-v1)

def fithr2(run,tdc,asic,direc,vthmin,vthmax,debug=False,draw=False):
  pedestal=[]
  max1=[]
  max2=[]
  outd= "output/run%d/pmr%d" % (run,tdc)
  os.system("mkdir -p "+outd)   
  rb=1
  fi=0
  la=63
  ped=[]
  for i in range(64):
    ped.append(0)
    pedestal.append(0)
    max1.append(0)
    max2.append(0)
  f82=TFile("/tmp/histo%d_0.root" % run);
  f82.cd("/pmr/D%d/A%d/%s" % (tdc,asic,direc));
  c1=None
  if (draw):
      c1=TCanvas("c1","Pedestal",900,900);
      print("Creqting c1")
  #c2=TCanvas("c2","Test",1400,900);
  #c2.cd()
  #c2.Divide(6,4)
  #c2.Draw()
  #c2.Update()
  #val = input()
  #c2.Draw()
  fout=open(outd+"/summary_pedestal.txt","w");
  fout.write("+--+-----+-----+-----+ \n");
  gStyle.SetOptFit();
  ncha=64
  hmean=TH1F("hmean","Summary %d %d " %(run,tdc),vthmax-vthmin+1,vthmin,vthmax)

  hnoise=TH1F("hnoise","Summary noise %d %d %d" %(run,tdc,asic),100,0.,10.)
  hpmean=TH1F("hpmean","Summary %d %d %d " %(run,tdc,asic),1*ncha,0.,1.*ncha);
  hpmax=TH1F("hpmean","Summary max %d %d %d " %(run,tdc,asic),1*ncha,0.,1.*ncha);
  hpnoise=TH1F("hpnoise","Summary noise %d %d %d" %(run,tdc,asic),1*ncha,0.,1.*ncha);
#  scfit=TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",vthmin+1,vthmax);
  scfit=TF1("scfit","gaus",vthmin+1,vthmax);
  
  for ip in range(la,fi,-1):
      #c2.cd()
      hs=None

      hs=f82.Get("/pmr/D%d/A%d/%s/Padc%d" % (tdc,asic,direc,ip));
      print("/pmr/D%d/A%d/%s/Padc%d" % (tdc,asic,direc,ip));
      if (hs==None):
          continue;
      if (hs.GetEntries()==0):
        continue
      print(ip,fi,la," found")
      hs.Scale(1./10.);
      nmax=0
      for i in range(1,hs.GetNbinsX()):
        if (hs.GetBinContent(i)==0):
              if (hs.GetBinContent(i-1)!=0 and hs.GetBinContent(i+1)!=0):
                  hs.SetBinContent(i,(hs.GetBinContent(i-1)+hs.GetBinContent(i+1))/2.)
        else:
          if (hs.GetBinContent(i)>nmax):
              nmax=hs.GetBinContent(i)

      hs.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      icolor= ip%4 +1
      istyle= ip/4+1
      hs.SetLineColor(icolor)
      hs.SetLineStyle(int(istyle))
      hs.SetLineWidth(2)
      if (draw):
          c1.cd()
          c1.Draw()
      



      if (ip==63):
        hs.Draw()
      else:
        hs.Draw("SAME")
  if (draw):
      c1.Update()
      #c1.SaveAs("Run%d_AllStrip%d.root" % (run,tdc,asic));
      c1.SaveAs(outd+"/AllStripR%d_D%d_A%d.pdf" % (run,tdc,asic));
      if (debug):
          val = input()
      else:
          time.sleep(2)
  vg=[128 for i in range(64)]
  nhmax=0
  for ip in range(fi,la+1):
      #c2.cd()

      hs=None
      hs=f82.Get("/pmr/D%d/A%d/%s/Padc%d" % (tdc,asic,direc,ip));

      if (hs==None):
          continue;
      if (hs.GetEntries()==0):
        continue
      #hs.Scale(1./2700.);
      hder=TH1F("hder%d" % ip,"derivative",int(400/rb),0.,400.)	
      #hs.Rebin(rb)
      #hs.Smooth()
      vmax=0
      for i in range(1,hs.GetNbinsX()-1):
          if (hs.GetBinContent(i)==0):
             if (hs.GetBinContent(i-1)!=0 and hs.GetBinContent(i+1)!=0):
                 hs.SetBinContent(i,(hs.GetBinContent(i-1)+hs.GetBinContent(i+1))/2.)
          else:
              if (hs.GetBinContent(i)>3 and hs.GetBinCenter(i) <vthmax):
                  vmax=hs.GetBinCenter(i)
              #print(i,hs.GetBinContent(i),vmax,hs.GetNbinsX()-1)
      nhigh=0
      for i in range(1,hs.GetNbinsX()-1):
          if (hs.GetBinCenter(i) >vmax+5):
                  nhigh=nhigh+hs.GetBinContent(i)
              #print(i,hs.GetBinContent(i),vmax,hs.GetNbinsX()-1)

      for i in range(1,hs.GetNbinsX()):
        if (hs.GetBinContent(i)-hs.GetBinContent(i+1)>-10):
            hder.SetBinContent(i,hs.GetBinContent(i)-hs.GetBinContent(i+1))
        else:
            hder.SetBinContent(i,-10)  
            #hder.Rebin(2)
      hder.Smooth()
      hder.GetXaxis().SetRangeUser(vthmin-1,vthmax);
      nmax=0
      xmax=0
      for i in range(1,hder.GetNbinsX()):
          if (hder.GetBinContent(i)>nmax):
              nmax=hder.GetBinContent(i)
              xmax=hder.GetBinCenter(i)
      hder.GetXaxis().SetRangeUser(xmax-5,xmax+15);
      scfit.SetParameter(0,nmax*0.7);
      scfit.SetParameter(1,xmax);
      scfit.SetParameter(2,1.);
      #scfit.SetParameter(2,5.);



      hs.GetXaxis().SetRangeUser(vthmin+1,vthmax);
      hder.Fit("scfit","Q","",xmax-5,xmax+5);
      #hs.GetXaxis().SetRangeUser(vthmin-1,scfit.GetParameter(1)+60);
      #gPad.SetLogy();
      rped=scfit.GetParameter(1)
      rped=xmax
      sped=hder.GetRMS()
      if (sped==0):
          sped=1.
      vg[ip]=int(round(vg[ip]*0.5/sped))
      if (debug):
          print("heho ",ip,rped,hder.GetMean(),scfit.GetParameter(2),xmax,vmax)
          if (nhigh>nhmax):
              nhmax=nhigh
              print("\t %d %d %d \n" % (ip,nhigh,nhmax))
      pedestal[ip]=rped
      max1[ip]=vmax
      max2[ip]=int(nhigh*100)*0.01
      if (debug and draw):
          c1.cd()
          c1.Draw()
      

          hder.Draw()
          c1.Update()

          val1 = input()          

      
          rped=hder.GetMean()
      
          hs.Draw()
      

          c1.cd()
          c1.Draw()
          c1.Update()
          val1 = input()          

      fout.write("|%2d|%5.1f|%5.1f|%5.2f| \n" % (ip,rped,scfit.GetParameter(2),vmax));
      hmean.Fill(rped)
      hpmax.SetBinContent(ip+1,vmax)
      hnoise.Fill(scfit.GetParameter(2))
      hpmean.SetBinContent(ip+1,rped);
      hpnoise.SetBinContent(ip+1,scfit.GetParameter(2))
      #c1.SaveAs("Run%d_Strip%d.root" % (run,ip));
      #val = input()

      #hder.Draw()
      
      #c1.Update()
      #val = input()
  if (draw):
      c1.Clear()
      c1.Divide(2,2)
      c1.cd(1)
  
      hmean.Draw()
      hpmean.GetYaxis().SetRangeUser(vthmin,vmax+10)
      #hpmax.GetYaxis().SetRangeUser(vthmin,vmax+10)
      print (hpmax.GetMaximum())
      hpmean.Draw()
      c1.Update()
      #c1.SaveAs(outd+"/Pedestal.pdf");
      #val = input()
      c1.cd(2)
      hnoise.Draw()
      c1.Update()
      #c1.SaveAs(outd+"/NoiseInt.pdf");
      #val = input()
      c1.cd(3)
      hpnoise.Draw()
      c1.Update()
      #val = input()
      c1.Update()
      #c1.SaveAs(outd+"/Noise.pdf");
      c1.cd(4)
      hpmax.Draw()

      c1.Update()
      if (debug):
          val = input()
      else:
          time.sleep(2)
          c1.Update()
          c1.SaveAs(outd+"/SummaryR%d_D%d_A%d.pdf" % (run,tdc,asic));
          c1.Close()
  fout.write("+--+-----+-----+-----+ \n");
  fout.close()
  #print("vg=",vg)
  return (pedestal,max1,max2)
def fitahr2(run,tdc,direc,vthmin,vthmax,nasic=24):
    vm=[]
    for i in range(1,nasic+1):
        v=fithr2(run,tdc,i,direc,vthmin,vthmax)
        vm.append(v)
    print(vm)
def fitgain(run,dif,asic,direc="GCURVE",debug=False,draw=False):
  outd= "output/run%d/pmr%d/asic%d" % (run,dif,asic)
  os.system("mkdir -p "+outd)   
  rb=1
  fi=0
  la=63
  gf=[]
  for i in range(64):
    gf.append(255)
  f82=TFile("/tmp/histo%d_0.root" % run);
  f82.cd("/pmr/D%d/A%d/%s" % (dif,asic,direc));
  c1=TCanvas();
  #c2=TCanvas("c2","Test",1400,900);
  #c2.cd()
  #c2.Divide(6,4)
  #c2.Draw()
  #c2.Update()
  #val = input()
  #c2.Draw()
  fout=open(outd+"/summary_gain.txt","w");
  fout.write("+--+-----+-----+-----+ \n");
  gStyle.SetOptFit();
  ncha=64
  hmin=TH1F("hmin","Summary %d %d %d " %(run,dif,asic),255,0,255.)

 
  hpmin=TH1F("hpmin","Summary %d %d %d " %(run,dif,asic),ncha+3,0.,1.*(ncha+3));
  gf=[]
  for ip in range(0,64):
      #c2.cd()
      gf.append(255)
      hs=None

      hs=f82.Get("/pmr/D%d/A%d/%s/Padc%d" % (dif,asic,direc,ip));
      if (hs==None):
          continue;
      if (hs.GetEntries()==0):
        continue
      hs.Rebin(4)
      hs.Smooth(1)
      print(ip,fi,la," found")

      nmin=256
      for i in range(1,hs.GetNbinsX()):
          if (hs.GetBinContent(i)>2):
              nmin=hs.GetBinCenter(i)
              break
      if (nmin==256):
          nmin=133

      hpmin.SetBinContent(ip+1,nmin)
      hmin.Fill(nmin)
      gf[ip]=nmin-5
      fout.write("|%2d|%5.1f| \n" % (ip,nmin*1.));
      if (draw):
          icolor= ip%4 +1
          istyle= ip/4+1
          hs.SetLineColor(icolor)
          hs.SetLineStyle(int(istyle))
          hs.SetLineWidth(2)
          c1.cd()
          c1.Draw()
      



          if (ip==0):
              hs.Draw("HIST")
          else:
              hs.Draw("HIST")
              c1.Update()
          val=input()
  #c1.SaveAs("Run%d_AllStrip%d.root" % (run,tdc,asic));
  if (draw):
      c1.cd()
      c1.Divide(1,2)
      c1.cd(1)
      hmin.Draw()
      c1.Draw()
      c1.Update()
      c1.cd(2)
      hpmin.Draw()
      c1.Draw()
      c1.Update()
      
      val=input()
      c1.SaveAs(outd+"/AllGain.pdf");
      val = input()
  c1.cd()
  c1.Draw()
      



  
  hpmin.Draw()
  c1.Update()
  #c1.SaveAs("Run%d_AllStrip%d.root" % (run,tdc,asic));
  c1.SaveAs(outd+"/MinGainvschannel.pdf");
  print(gf)

  fout.write("+--+-----+-----+-----+ \n");
  fout.close()
  return gf
import time
def fitallgain(run,thr=120,dirs="B10SCURVE"):
    fout=open("summary_gain%d.py" % run,"w");
    gricv0=[32,33,34,35,36,37,42,43,44,45,46,47,52,53,54,55,56,57,
         72,73,74,75,76,77,82,83,84,85,86,87,12,13,14,16,17]
    gricv1=[11,12,13,21,22,23,31,32]
    for i in gricv1:
        pl=i/10
        dr=i%10
        dif=(pl*16+dr)*256+100
        pag=fitgain(run,i)
        fout.write("pag_%d=[" % i);
        for p in range(63):
            if (pag[p]<=0):
                pag[p]=128
            if (pag[p]>255):
                pag[p]=128
            fout.write("%d," % pag[p])
        fout.write("%d]\n" % pag[63])
        fout.write("s.HR2_SetAsicPAGain(%d,0,pag_%d)\n" % (dif,i))
        fout.write("s.HR2_ChangeThreshold(%d,512,512,%d)\n" % (thr,dif))
        time.sleep(1)
    fout.close()
def fitallhr2(run,thr=120,dirs="B10SCURVE"):
    gricv1=[11,12,13,21,22,23,31,32]
    for i in gricv1:
        pl=i/10
        dr=i%10
        dif=(pl*16+dr)*256+100
        pag=fithr2(run,i,"%s" % dirs,75,150)

        time.sleep(1)

def fitchamber(run,thmin,thmax,difd,difc,difg,dirs="B01SCURVE",chamber=1,slot=1):
    rd=fitdifped(run,difd,thmin,thmax,shiftJ=0)
    rc=fitdifped(run,difc,thmin,thmax,shiftJ=32)
    rg=fitdifped(run,difg,thmin,thmax,shiftJ=64)
    rt={}
    rt["info"]={}
    rt["info"]["difs"]=[difd,difc,difg]
    rt["info"]["run"]=run
    
    rt["droite"]=rd
    rt["centre"]=rc
    rt["gauche"]=rg
    srt=json.dumps(rt)
    with open('pedrun%d_ch%d_sl%d.json' % (run,chamber,slot), 'w') as f:
        f.write(srt)
    dif=1
    hped=TH2F("ped%d" % dif,"ped%d" % dif,96,0.,96.,96,0.,96.)
    hmax1=TH2F("max1%d" % dif,"max1%d" % dif,96,0.,96.,96,0.,96.)
    hmax2=TH2F("max2%d" % dif,"max2%d" % dif,96,0.,96.,96,0.,96.)
    for i in range(48):
        for j in range(64):
            fid,fjd=SdhcalConvert(i+1,j,shiftj=0,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fid,fjd,rd[i+1]["pedestal"][j])
            hmax1.SetBinContent(fid,fjd,rd[i+1]["max1"][j])
            hmax2.SetBinContent(fid,fjd,rd[i+1]["max2"][j])
            fic,fjc=SdhcalConvert(i+1,j,shiftj=32,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fic,fjc,rc[i+1]["pedestal"][j])
            hmax1.SetBinContent(fic,fjc,rc[i+1]["max1"][j])
            hmax2.SetBinContent(fic,fjc,rc[i+1]["max2"][j])
            fig,fjg=SdhcalConvert(i+1,j,shiftj=64,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fig,fjg,rg[i+1]["pedestal"][j])
            hmax1.SetBinContent(fig,fjg,rg[i+1]["max1"][j])
            hmax2.SetBinContent(fig,fjg,rg[i+1]["max2"][j])
        vpd=rd[i+1]["pedestal"]
        vpd.sort()
        nv=np.array(vpd)
        print(i+1,np.mean(nv),np.median(nv),stats.trim_mean(rd[i+1]["pedestal"], 0.1))
    c1=TCanvas("c1","Pedestal",900,900);
    print("Creqting c1")
    c1.cd()
    c1.Draw()
      



  
    hped.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax1.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax2.Draw("COLZ")
    c1.Update()
    
    val = input()
    outHistFile=TFile.Open('pedrun%d_ch%d_sl%d.root' % (run,chamber,slot),"RECREATE")
    outHistFile.cd()
    hped.Write()
    hmax1.Write()
    hmax2.Write()
    outHistFile.Close()
def fitdifped(run,dif,thmin,thmax,dirs="B01SCURVE",shiftJ=0):
    sj=shiftJ
    res=[]
    res.append({})
    hped=TH2F("ped%d" % dif,"ped%d" % dif,96,0.,96.,96,0.,96.)
    hmax1=TH2F("max1%d" % dif,"max1%d" % dif,96,0.,96.,96,0.,96.)
    hmax2=TH2F("max2%d" % dif,"max2%d" % dif,96,0.,96.,96,0.,96.)
    for i in range(48):
        res.append({})
        pag=fithr2(run,dif,i+1,"%s" % dirs,thmin,thmax,debug=False,draw=False)
        res[i+1]["pedestal"]=pag[0]
        res[i+1]["max1"]=pag[1]
        res[i+1]["max2"]=pag[2]
        for j in range(64):
            fi,fj=SdhcalConvert(i+1,j,shiftj=sj,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fi,fj,pag[0][j])
            hmax1.SetBinContent(fi,fj,pag[1][j])
            hmax2.SetBinContent(fi,fj,pag[2][j])
            #val=input()
    #print(res)
    #time.sleep(1)
    c1=TCanvas("c1","Pedestal",900,900);
    print("Creqting c1")
    c1.cd()
    c1.Draw()
      



  
    hped.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax1.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax2.Draw("COLZ")
    c1.Update()
    
    val = input()
    return res

def fitdifgain(run,dif,shiftJ=0):
    sj=shiftJ
    res=[]
    res.append({})
    hped=TH2F("gain%d" % dif,"ped%d" % dif,96,0.,96.,96,0.,96.)
    for i in range(48):
        res.append({})
        pag=fitgain(run,dif,i+1,debug=False,draw=False)
        res[i+1]["gain"]=pag
        for j in range(64):
            fi,fj=SdhcalConvert(i+1,j,shiftj=sj,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fi,fj,pag[j])
            #val=input()
    #print(res)
    #time.sleep(1)
    c1=TCanvas("c1","Pedestal",900,900);
    print("Creqting c1")
    c1.cd()
    c1.Draw()
      



  
    hped.Draw("COLZ")
    c1.Update()
    
    #val = input()
    return res


def SdhcalConvert(asicid,ipad,poli=1,polj=1,shifti=0,shiftj=0):
    MapJLargeHR2=[1,1,2,2,3,3,4,4,5,5,6,6,7,7,4,3,2,0,0,1,0,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,0,0,1,1,0,2,3,4,5,7,7,6,6,5,5,4,4,3,3,2,2,1]
    MapILargeHR2=[1,0,1,0,1,0,1,0,0,1,0,1,0,1,2,2,2,0,1,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,6,5,6,7,5,5,5,5,6,7,6,7,6,7,6,7,7,6,7,6,7]
    AsicShiftI=[0,0,0,0,0,8,8,8,8,16,16,16,16,24,24,24,24,32,32,32,32,40,40,40,40,48,48,48,48,56,56,56,56,64,64,64,64,72,72,72,72,80,80,80,80,88,88,88,88]
    AsicShiftJ=[0,0,8,16,24,24,16,8,0,0,8,16,24,24,16,8,0,0,8,16,24,24,16,8,0,0,8,16,24,24,16,8,0,0,8,16,24,24,16,8,0,0,8,16,24,24,16,8,0]


    I=0
    J=0
    pad=1.04125;
  
    I = MapILargeHR2[ipad]+AsicShiftI[asicid]+1;
    J = MapJLargeHR2[ipad]+AsicShiftJ[asicid]+1;
    J=33-J;
  

    #from DIF (I,J) to chamber
    fI=0
    fJ=0
    if (poli>0):
        fI=I+shifti
    else:
        fI=(96-I+1)+shifti;
    if (polj>0):
        fJ=J+shiftj;
    else:
        fJ=(32-J+1)+shiftj;

    return (fI,fJ)

def jsonchamber(fname):
    rt=json.load(open("%s.json" % fname))
    rd=rt["droite"]
    rc=rt["centre"]
    rg=rt["gauche"]
    dif=1
    hped=TH2F("ped%d" % dif,"ped%d" % dif,96,0.,96.,96,0.,96.)
    hmax1=TH2F("max1%d" % dif,"max1%d" % dif,96,0.,96.,96,0.,96.)
    hmax2=TH2F("max2%d" % dif,"max2%d" % dif,96,0.,96.,96,0.,96.)
    hmpd=TH1F("mean_ped_droite","mean_ped_droite",48,0.1,48.1)
    hmpc=TH1F("mean_ped_centre","mean_ped_centre",48,0.1,48.1)
    hmpg=TH1F("mean_ped_gauche","mean_ped_gauche",48,0.1,48.1)
    for i in range(48):
        mtd=stats.trim_mean(rd[i+1]["pedestal"], 0.1)
        mtc=stats.trim_mean(rc[i+1]["pedestal"], 0.1)
        mtg=stats.trim_mean(rg[i+1]["pedestal"], 0.1)
        for j in range(64):
            fid,fjd=SdhcalConvert(i+1,j,shiftj=0,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fid,fjd,rd[i+1]["pedestal"][j]-mtd)
            hmax1.SetBinContent(fid,fjd,rd[i+1]["max1"][j]-mtd)
            hmax2.SetBinContent(fid,fjd,rd[i+1]["max2"][j])
            fic,fjc=SdhcalConvert(i+1,j,shiftj=32,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fic,fjc,rc[i+1]["pedestal"][j]-mtc)
            hmax1.SetBinContent(fic,fjc,rc[i+1]["max1"][j]-mtc)
            hmax2.SetBinContent(fic,fjc,rc[i+1]["max2"][j])
            fig,fjg=SdhcalConvert(i+1,j,shiftj=64,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fig,fjg,rg[i+1]["pedestal"][j]-mtg)
            hmax1.SetBinContent(fig,fjg,rg[i+1]["max1"][j]-mtg)
            hmax2.SetBinContent(fig,fjg,rg[i+1]["max2"][j])
        vpd=rd[i+1]["pedestal"]
        vpd.sort()
        nv=np.array(vpd)
        hmpd.SetBinContent(i+1,stats.trim_mean(rd[i+1]["pedestal"], 0.1))
        hmpc.SetBinContent(i+1,stats.trim_mean(rc[i+1]["pedestal"], 0.1))
        hmpg.SetBinContent(i+1,stats.trim_mean(rg[i+1]["pedestal"], 0.1))
        print(i+1,np.mean(nv),np.median(nv),stats.trim_mean(rd[i+1]["pedestal"], 0.1))
    c1=TCanvas("c1","Pedestal",900,900);
    print("Creqting c1")
    c1.cd()
    c1.Draw()
      



  
    hped.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax1.Draw("COLZ")
    c1.Update()
    
    val = input()
    c1.cd()
    c1.Draw()
    hmax2.Draw("COLZ")
    c1.Update()
    
    val = input()
    outHistFile=TFile.Open("%s.root" % fname,"RECREATE")
    outHistFile.cd()
    hped.Write()
    hmax1.Write()
    hmax2.Write()
    hmpd.Write()
    hmpc.Write()
    hmpg.Write()
    outHistFile.Close()

def gainChambers(run,chamberlist,name,comment):
    setup=os.getenv("DAQSETUP","NONE")
    if (setup == "NONE"):
        print("DAQSETUP should be set")
        exit(0)
    geom=os.getenv("GEOMSETUP","NONE")
    if (geom == "NONE"):
        print("GEOMSETUP should be set")
        exit(0)
    fg=open(geom)
    sg=json.load(fg)
    res={}
    res["setup"]=setup
    res["chambers"]=[]
    for ch in chamberlist:
        for x in sg["chambers"]:
            if (x["num"]!=ch):
                continue
            d=0;c=0;g=0;
            for y in sg["difs"]:
                if (y["chamber"]!=ch):
                    continue
                if (int(y["dj"])==0):
                    d=y["id"]
                if (int(y["dj"])==32):
                    c=y["id"]
                if (int(y["dj"])==64):
                    g=y["id"]
            if (d==0 or c==0 or g==0):
                continue
            print("chamber %d with difs %d,%d,%d \n" % (ch,d,c,g))
            val=input()
            rc=fitgainchamber(run,d,c,g,chamber=ch)
            res["chambers"].append(rc)
    srt=json.dumps(res)
    with open('gains_%s_%d.json' % (setup,run), 'w') as f:
        f.write(srt)
    _wdd.uploadCalibration(name,res,"GAIN",comment)
            
def fitgainchamber(run,difd,difc,difg,chamber=1):
    rd=fitdifgain(run,difd,shiftJ=0)
    rc=fitdifgain(run,difc,shiftJ=32)
    rg=fitdifgain(run,difg,shiftJ=64)
    rt={}
    rt["info"]={}
    rt["info"]["chamber"]=chamber
    rt["info"]["difs"]=[difd,difc,difg]
    rt["info"]["run"]=run
    
    rt["droite"]=rd
    rt["centre"]=rc
    rt["gauche"]=rg
    srt=json.dumps(rt)
    if (chamber==1):
        with open('chambergain%d.json' % run, 'w') as f:
            f.write(srt)
    dif=1
    hped=TH2F("gain%d" % dif,"gain%d" % chamber,96,0.,96.,96,0.,96.)

    for i in range(48):
        for j in range(64):
            fid,fjd=SdhcalConvert(i+1,j,shiftj=0,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fid,fjd,rd[i+1]["gain"][j])
            fic,fjc=SdhcalConvert(i+1,j,shiftj=32,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fic,fjc,rc[i+1]["gain"][j])
            fig,fjg=SdhcalConvert(i+1,j,shiftj=64,polj=1)
            #print(i+1,j,fi,fj)
            hped.SetBinContent(fig,fjg,rg[i+1]["gain"][j])
        vpd=rd[i+1]["gain"]
        vpd.sort()
        nv=np.array(vpd)
        print(i+1,np.mean(nv),np.median(nv),stats.trim_mean(rd[i+1]["gain"], 0.1))
    c1=TCanvas("c1","Gain",900,900);
    print("Creqting c1")
    c1.cd()
    c1.Draw()
      



  
    hped.Draw("COLZ")
    c1.Update()
    
    val = input()
    outHistFile=TFile.Open("sumgain%d.root" % run,"RECREATE")
    outHistFile.cd()
    hped.Write()

    outHistFile.Close()
    return rt
