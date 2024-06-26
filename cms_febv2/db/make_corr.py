"""
Two class to analyze FEBV2 commissioning tests
"""
import json
import csv_register_access as cra
import ROOT
import numpy as np
from scipy import stats
class pedcor:
    """
    Class to analyze DAC10 bit scan of PETIROC and calculate DAC6bit correction per channel and DAC10bits threshold per ASIC
    """
    def __init__(self,state,version,feb,analysis):
        """ Initialise the analysis and download a test result

        Args:
            state(str): Setup state used for the test
            version(int): Setup state version
            feb(int): Feb id of the test
            analysis(str): Name of the analysis SCURVE_1 or SCURVE_A
        """
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
        """ Draw all Scurves of the test in ROOT TCanvas

        Args:
            save(bool): Save PDF (False by default)
            debug(bool): Draw Scurves one by one (False by default)
        """
        for a in self.scurves.keys():
            self.draw_scurves(self.scurves[a],save,debug)
    def draw_asic(self,asic="MIDDLE_TOP",title=None):
        """ Draw all Scurves of the test in ROOT TCanvas

        Args:
            save(bool): Save PDF (False by default)
            debug(bool): Draw Scurves one by one (False by default)
        """
        for a in self.scurves.keys():
            self.draw_one_asic(self.scurves[a],a_name=asic,title=title)

    def draw_scurves(self,d_sc,save,debug):
        """ Draw all Scurves of one asic in ROOT format

        Args:
            d_sc:JSON object stored in the febv2_test collection
            save(bool): Save summary histograms to PDF
            debug(bool): Fit each scurve one by one
        Returns:
            A list of ROOT.TH1F histos containing the SCURVES
        """
        asic=d_sc["asic"]
        analysis=d_sc["analysis"]
        state=d_sc["state"]
        version=d_sc["version"]
        feb=d_sc["feb"]
        thi=d_sc["thmin"]
        tha=d_sc["thmax"]
        fn=f"{analysis}_{state}-v{version}-f{feb}"
        c1=ROOT.TCanvas("SCurves","Scurves",600,900)
        c2=ROOT.TCanvas("Fit")
        icol=1
        histos=[]
        thrs=[]
        thre=[]
        thrm=[]
        thrr=[]
        v_thmax=[]
        v_thmin=[]
        all_lines=[]
        scfit=ROOT.TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",thi+1,tha);
        ROOT.gStyle.SetOptStat(0)
        hpmean=ROOT.TH1F("hpmean",f"Summary pedestal {fn} {asic} ",16,0.,16.0)
        hpnoise=ROOT.TH1F("hpnoise",f"Summary Noise {fn} {asic} ",16,0.,16.0)
        hpmean.GetYaxis().SetRangeUser(thi,tha)
        c1.Divide(1,3)
        c1.cd(1)
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
            ithmax=0
            ithmin=9999
            for ith in range(nval):
                if (vals[ith]<0.8*vmax and ithmax==0):
                    ithmax=ith+thi
                if (vals[ith]<0.2*vmax and ithmin==9999):
                    ithmin=ith+thi
                hs.SetBinContent(ith+1,vals[ith])
                if (ith-1>0):
                    if (vals[ith-1]-vals[ith]>-10):
                        hd.SetBinContent(ith,vals[ith-1]-vals[ith])
            c1.cd(1)
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
            thrm.append(hd.GetMean())
            thrr.append(hd.GetRMS())
            v_thmax.append(ithmax)
            v_thmin.append(ithmin)
            c2.Draw()
            c2.Update()
            print(f"Pedestal {hd.GetMean()} Noise {hd.GetRMS()}")
            if (debug):
                v=input()
                
            hd.Draw()
            c2.Draw()
            c2.Update()

            if (debug):
                v=input()
            

            histos.append(hs)
        print(thrs)
        print(thre)
        hpmean.Reset()
        for i in range(len(thrs)):
            # Bad gaussian take 0.2-0.8 width
            if (thrr[i]>(v_thmin[i]-v_thmax[i]+1)):
                thrr[i]=(v_thmin[i]-v_thmax[i]+1)
            print(f"channel {i} ++Erf Fit++ {thrs[i]:.1f} Noise {thre[i]:.1f} ++ Gaussian ++ {thrm[i]:.1f} {thrr[i]:.1f} ++ 80 % ++ {v_thmax[i]:.1f} {v_thmin[i]:.1f} ")
            # Bad fit take the gaussian mean and RMS
            if (thre[i]>3*thrr[i]):
                thrs[i]=thrm[i]
                thre[i]=thrr[i]
            thr=thrs[i]+5*thre[i]

            hpmean.SetBinContent(i+1,thrs[i])
            hpnoise.SetBinContent(i+1,thre[i])
        c2.Close()
        del c2
        if (save):

            c1.cd(2)
            hpmean.Draw()
            c1.Draw()
            c1.Update()
            
            np_thrm=np.array(thrs)
            np_thrr=np.array(thre)
            seuil=np_thrm.max()+5*np_thrr.max()
            print(np_thrm.max(),np_thrr.max(),seuil,thi,tha)
            tl=ROOT.TLine(0,seuil,16,seuil)
            tl.SetLineColor(2)
            tl.Draw("SAME")
            tt=ROOT.TLatex()
            #tt=ROOT.TText(8,seuil+20,f"T_{{max}} {np_thrm.max():.1f} Noise_{{max}} {np_thrr.max():.1f} VTH cut {seuil:.1f}")
            tt.SetTextAlign(22);
            tt.SetTextColor(ROOT.kBlue+2);
            #tt.SetTextFont(33);
            tt.SetTextSize(0.05);
            s_lat=f"T_{{max}} {np_thrm.max():.1f} Noise_{{max}} {np_thrr.max():.1f} VTH cut {seuil:.1f}";
            print(s_lat)
            tt.DrawLatex(8,seuil+20,s_lat)
            c1.Draw()
            c1.Update()
            v=input()
            c1.cd(3)
            hpnoise.Draw()
            c1.Draw()
            c1.Update()

            c1.cd()
            c1.SaveAs(f"results/{fn}_{asic}.pdf")
            c1.SaveAs(f"results/{fn}_{asic}.bmp")
            v=input()
        v=input("Next ASIC?")
        return histos
    def full_threshold(self,c_upload=None):
        """ Calculate DAC6b shift per channel and DAC10b thresholds for all asics and upload to DB collection
            febv2_setup if required

        Args:
            c_upload(str): If set , upload to DB with this comment
        """
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
        """ Calculate turn on for one ASIC
        Args:
            asic(str): Asic name (LEFT_BOT...RIGHT_TOP)
        Returns:
            res an array of channels tuple (ch,turnon,crrection,mean asic turnon)
        """
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
    def draw_one_asic(self,d_sc,a_name,title):
        """ Draw all Scurves of one asic in ROOT format

        Args:
            d_sc:JSON object stored in the febv2_test collection
            a_name: Asic name
        Returns:
            A list of ROOT.TH1F histos containing the SCURVES
        """
        asic=d_sc["asic"]
        if (asic!=a_name):
            return []

        analysis=d_sc["analysis"]
        state=d_sc["state"]
        version=d_sc["version"]
        feb=d_sc["feb"]
        thi=d_sc["thmin"]
        tha=d_sc["thmax"]
        fn=f"{analysis}_{state}-v{version}-f{feb}"
        c2=ROOT.TCanvas("Scurves")
        icol=1
        histos=[]
        ROOT.gStyle.SetOptStat(0)
        c2.Clear()
        c2.Divide(1,1)
        c2.cd(1)
        vm=-1
        for c in d_sc["channels"]:
            ch=c["prc"]
            vals=c["scurve"]
            nval=len(vals)        
            vmax=max(vals)
            if (vmax>vm):
                vm=vmax
        for c in d_sc["channels"]:
            ch=c["prc"]
            vals=c["scurve"]
            nval=len(vals)
            shs="%s_%s_scurve-c%d" % (fn,asic,ch)
            hs=ROOT.TH1F(shs,shs,tha-thi+1,thi,tha)
            diff=[0 for x in range(nval)]
            vmax=max(vals)
            ithmax=0
            ithmin=9999
            for ith in range(nval):
                hs.SetBinContent(ith+1,vals[ith])
            c2.cd(1)
            hs.SetLineColor(icol)

            if (icol==1):
                if (title!=None):
                    hs.SetTitle(title)
                hs.GetYaxis().SetRangeUser(0,vm*1.07)
                hs.Draw()
            else:
                hs.Draw("SAME")
            icol=icol+1
            c2.Draw()
            c2.Update()
            histos.append(hs)
        c2.cd()
        c2.Draw()
        c2.Update()
        v=input()
        c2.SaveAs(f"results/{fn}_{asic}.bmp")
        c2.SaveAs(f"results/{fn}_{asic}.pdf")
        v=input()
        return histos

class timecor:
    """
    Class to analyze DAC10 bit scan of PETIROC and calculate DAC6bit correction per channel and DAC10bits threshold per ASIC
    """
    def __init__(self,state,version,feb,analysis):
        """ Initialise the analysis and download a test result
        
        Args:
            state(str): Setup state used for the test
            version(int): Setup state version
            feb(int): Feb id of the test
            analysis(str): Name of the analysis TIME_PEDESTAL
        """
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
        """ Draw all time pedestals of the test in ROOT TCanvas

        Args:
            save(bool): Save PDF (False by default)
            debug(bool): Draw pedestals one by one (False by default)
        """
        for a in self.pedestals.keys():
            self.draw_pedestals(self.pedestals[a],save,debug)
    def draw_pedestals(self,d_sc,save,debug):
        """ Draw all pedestals of one fpga in ROOT format

        Args:
            d_sc:JSON object stored in the febv2_test collection
            save(bool): Save summary histograms to PDF
            debug(bool): draw each tdc channel one by one
        Returns:
            A list of ROOT.TH1F histos containing the channel pedestals
        """
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
            c1.SaveAs(f"results/{fn}_{asic}.bmp")
        v=input("Next FPGA?")
        return histos
    def full_pedestals(self,c_upload=None,mapping=None):
        """ Calculate pedestals for all channels and upload TS_OFFSET if required
        Args:
            c_upload(str): None, if set the DB is upload with this comment
        
        """
        if (mapping!=None):
            sm=json.load(open(mapping))
            
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
        min_del=0
        for ch in range(16):
            print("{:<10} {:<10} {:<10} {:<10}".format(ch,fpga_chan_offset['LEFT'][ch],fpga_chan_offset['LEFT'][31-ch],fpga_chan_offset['LEFT'][ch]-fpga_chan_offset['LEFT'][31-ch]))
            min_del=min_del+fpga_chan_offset['LEFT'][ch]-fpga_chan_offset['LEFT'][31-ch]
        print(min_del/16.)

        min_del=0
        for ch in range(16):
            print("{:<10} {:<10} {:<10} {:<10}".format(ch,fpga_chan_offset['MIDDLE'][ch],fpga_chan_offset['MIDDLE'][31-ch],fpga_chan_offset['MIDDLE'][ch]-fpga_chan_offset['MIDDLE'][31-ch]))
            min_del=min_del+fpga_chan_offset['MIDDLE'][ch]-fpga_chan_offset['MIDDLE'][31-ch]
        print(min_del/16.)
        min_del=0
        for ch in range(16):
            print("{:<10} {:<10} {:<10} {:<10}".format(ch,fpga_chan_offset['RIGHT'][ch],fpga_chan_offset['RIGHT'][31-ch],fpga_chan_offset['RIGHT'][ch]-fpga_chan_offset['RIGHT'][31-ch]))
            min_del=min_del+fpga_chan_offset['RIGHT'][ch]-fpga_chan_offset['RIGHT'][31-ch]
        print(min_del/16.)
        upload=(c_upload!=None)
        if (upload):
            cm=c_upload
            self.sdb.upload_changes(cm)
        else:
            self.sdb.setup.version=999
            self.sdb.setup.to_csv_files()
       
