import json
import csv_register_access as cra
import ROOT
import numpy as np
from scipy import stats
class pedcor:
    def __init__(self,state,version,feb,analysis):
        print(state,version)
        self.sdb=cra.instance()
        self.sdb.download_setup(state,version)
        self.scurves={}
        asicl=["left_top","left_bot","middle_top","middle_bot","right_top","right_bot"]
        for a in asicl:
            print(f"Getting Scurves for {a} from analysis {analysis} on {state}/{version}")
            self.scurves[a.upper()]=self.sdb.get_scurve(state,version,feb,analysis,a.upper())
        self.full_done=False
    def draw_all(self,save=False,debug=False):
        for a in self.scurves.keys():
            self.draw_scurves(self.scurves[a],save,debug)
    def draw_scurves(self,d_sc,save,debug):
        asic=d_sc["asic"]
        analysis=d_sc["analysis"]
        state=d_sc["state"]
        version=d_sc["version"]
        feb=d_sc["feb"]
        thi=d_sc["thmin"]
        tha=d_sc["thmax"]
        fn=f"{analysis}_{state}-v{version}-f{feb}"
        c1=ROOT.TCanvas()
        c2=ROOT.TCanvas("Fit")
        icol=1
        histos=[]
        thrs=[]
        thre=[]
        scfit=ROOT.TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",thi+1,tha);
        ROOT.gStyle.SetOptStat(0)
        for c in d_sc["channels"]:
            ch=c["prc"]
            vals=c["scurve"]
            nval=len(vals)
            shd="%s_%s_diff-c%d" % (fn,asic,ch)
            shs="%s_%s_scurve-c%d" % (fn,asic,ch)
            hd=ROOT.TH1F(shd,shd,tha-thi+1,thi,tha)
            hs=ROOT.TH1F(shs,shs,tha-thi+1,thi,tha)
            diff=[0 for x in range(nval)]
            vmax=max(vals)
            for ith in range(nval):
                hs.SetBinContent(ith+1,vals[ith])
                if (ith-1>0):
                    hd.SetBinContent(ith,vals[ith-1]-vals[ith])
            c1.cd()
            hs.SetLineColor(icol)

            if (icol==1):
                hs.Draw()
            else:
                hs.Draw("SAME")
            icol=icol+1
            c1.Draw()
            c1.Update()
            scfit.SetParameter(0,vmax/2);
            scfit.SetParameter(1,hd.GetMean());
            scfit.SetParameter(2,hd.GetRMS()/2.);
            c2.cd()
            hsc=hs.Clone()
            hsc.Fit("scfit","","");
            thrs.append(scfit.GetParameter(1))
            thre.append(scfit.GetParameter(2))
            c2.Draw()
            c2.Update()
            if (debug):
                v=input()
            histos.append(hs)
        print(thrs)
        print(thre)
        c2.Close()
        del c2
        if (save):
            c1.cd()
            c1.SaveAs(f"results/{fn}_{asic}.pdf")
        v=input("Next ASIC?")
        return histos
    def full_threshold(self,c_upload=None):
        upload=(c_upload!=None)
        if (self.full_done):
            print("already corrected \n")
            return
        self.full_done=True
        asicl=self.scurves.keys()
        self.thrs=[]
        for a in asicl:
            print("="*62)
            print(f"SCurves correction for asic {a}")
            print("="*62)
            r=self.find_thresholds(a)
            inserted=False
            for ch,th,cor,thop in r:
                self.sdb.setup.febs[0].petiroc.correct_6b_dac(ch,cor,a.upper())
                self.sdb.setup.febs[0].petiroc.set_parameter("10b_dac_vth_discri_time",thop,a.upper())
                if (not inserted):
                    self.thrs.append((a,thop))
                    inserted=True
        #self.sdb.setup.version=999
        #self.sdb.setup.to_csv_files()
        if (upload):
            cm=c_upload
            self.sdb.upload_changes(cm)
        else:
            self.sdb.setup.version=999
            self.sdb.setup.to_csv_files()
        #print(self.thrs)
        print("="*62)
        print("Turn On Summary")
        print("="*62)
        print ("{:<12} {:<10}".format('ASIC','Turn On'))
        for a,to in self.thrs:
            print ("{:<12} {:<10}".format(a,to))
        return
    def find_thresholds(self,asic):
        d_sc=self.scurves[asic]
        thi=d_sc["thmin"]
        tha=d_sc["thmax"]
        ths=d_sc["thstep"]
        thrs=[]
        thrds=[]
        for c in d_sc["channels"]:
            ch=c["prc"]
            vals=c["scurve"]
            nval=len(vals)
            vmax=0
            threshold=9000
            diff=[0 for x in range(nval)]
            for ith in range(nval):
                if (vals[ith]>vmax):
                    vmax=vals[ith]
                if (vals[ith]<0.8*vmax):
                    threshold=ith*ths+thi
                    break
            thrs.append((ch,threshold))
        thm=0
        for _,th in thrs:
            thm=thm+th
        thop=int(thm/len(thrs))
        res=[]
        print ("{:<8} {:<10} {:<10} {:<10}".format('Channel','Turn On','Shift','Goal'))
        for ch,th in thrs:
            cor=(thop-th)/3.9
            #print(ch,th,round(cor),thop)
            print ("{:<8} {:<10} {:<10} {:<10}".format(ch,th,round(cor),thop))
            res.append((ch,th,round(cor),thop))
            #print(f"sdb.correct_6bdac({ch},{round(cor)},{self.asic})")
        
        return res

class timecor:
    def __init__(self,state,version,feb,analysis):
        print(state,version)
        self.sdb=cra.instance()
        self.sdb.download_setup(state,version)
        self.pedestals={}
        fpgal=["LEFT","MIDDLE","RIGHT"]
        for f in fpgal:
            print(f"Getting time pedestals for {f} from analysis {analysis} on {state}/{version}")
            self.pedestals[f.upper()]=self.sdb.get_time_pedestal(state,version,feb,analysis,f.upper())
        #print(self.pedestals["RIGHT"])
        self.full_done=False
    def draw_all(self,save=False,debug=False):
        for a in self.pedestals.keys():
            self.draw_pedestals(self.pedestals[a],save,debug)
    def draw_pedestals(self,d_sc,save,debug):
        asic=d_sc["fpga"]
        analysis=d_sc["analysis"]
        state=d_sc["state"]
        version=d_sc["version"]
        feb=d_sc["feb"]
        fn=f"{analysis}_{state}-v{version}-f{feb}"
        # find histo limits
        t_min=0xFFFFFF
        t_max=0
        #print(d_sc["channels"])
        for ch in range(len(d_sc["channels"])):
            if (ch==32):
                continue
            vals=d_sc["channels"][ch]
            if (len(vals)>10):
                vmean=stats.trim_mean(vals, 0.1)
                v_min=min(vals)
                v_max=max(vals)
                v_min=vmean-1000
                v_max=vmean+1000
                if (v_min<t_min):
                    t_min=v_min
                if(v_max>t_max):
                    t_max=v_max
                    #print(np.mean(vals),stats.trim_mean(vals, 0.1))
                    print(ch," Max")
        print(t_min,t_max)
        dt=(t_max-t_min+1)//5
        nb=int((t_max-t_min+1)+2*dt)
        #nb=nb//5
        print("dt ",dt,"nb ",nb,t_min-dt,t_max+dt)
        xmin=t_min-1.*dt
        xmax=t_max+1*dt
        #return
                
        c1=ROOT.TCanvas()
        icol=1
        histos=[]
        ROOT.gStyle.SetOptStat(0)
        for ch in range(len(d_sc["channels"])):
            vals=d_sc["channels"][ch]
            nval=len(vals)
            print(nval)
            if (nval==0):
                continue
            shs="%s_%s_scurve-c%d" % (fn,asic,ch)
            hs=ROOT.TH1F(shs,shs,nb,xmin,xmax)
            for ith in range(nval):
                hs.Fill(vals[ith])
            c1.cd()
            hs.SetLineColor(icol)

            if (icol==1):
                #hs.GetYaxis().SetRangeUser(0.,200.)
                hs.Draw()
            else:
                hs.Draw("SAME")
            icol=icol+1
            c1.Draw()
            c1.Update()
            if (debug):
                v=input()
            histos.append(hs)

        if (save):
            c1.cd()
            c1.SaveAs(f"results/{fn}_{asic}.pdf")
        v=input("Next FPGA?")
        return histos
    def full_pedestals(self,c_upload=None):
        if (self.full_done):
            print("already corrected \n")
            return
        fpga_chan_offset = {}
        fpga_chan_mu = {}
        min_resync = 5000000000
        for fpga in self.pedestals.keys():
            fpga_chan_mu[fpga]=[0 for i in range(34)]
            for ch in range(34):
                if (ch!=32):
                    #fpga_chan_mu[fpga][ch] = np.mean(self.pedestals[fpga]["channels"][ch])
                    fpga_chan_mu[fpga][ch]=stats.trim_mean(self.pedestals[fpga]["channels"][ch], 0.1)
                else:
                    fpga_chan_mu[fpga][ch]=0
            min_resync = min(min_resync, fpga_chan_mu[fpga][33])
            fpga_chan_offset[fpga] = [0]*34
        #print(fpga_chan_mu)
        print(" aligned to channel lowest time[33]:", min_resync)
        for fpga in self.pedestals.keys():
            ##if (fpga.lower()!="right"):
            ##    continue
            for ch in range(34):
                if ch != 32: fpga_chan_offset[fpga][ch] = round(fpga_chan_mu[fpga][ch]-min_resync)

        upload=(c_upload!=None)
        print ("{:<10} {:<10} {:<10} {:<10}".format("Channel","LEFT","MIDDLE","RIGHT"))
        for ch in range(34):
            if ch < 32:
                print("{:<10} {:<10} {:<10} {:<10}".format(ch,fpga_chan_offset['LEFT'][ch], fpga_chan_offset['MIDDLE'][ch], fpga_chan_offset['RIGHT'][ch]))
                self.sdb.setup.febs[0].fpga.set_ts_offset(ch,fpga_chan_offset['LEFT'][ch],'LEFT')
                self.sdb.setup.febs[0].fpga.set_ts_offset(ch,fpga_chan_offset['MIDDLE'][ch],'MIDDLE')
                self.sdb.setup.febs[0].fpga.set_ts_offset(ch,fpga_chan_offset['RIGHT'][ch],'RIGHT')
            else:
                print("{:<10} {:<10} {:<10} {:<10}".format(ch,0,0,0))

        upload=(c_upload!=None)
        if (upload):
            cm=c_upload
            self.sdb.upload_changes(cm)
        else:
            self.sdb.setup.version=999
            self.sdb.setup.to_csv_files()
       
