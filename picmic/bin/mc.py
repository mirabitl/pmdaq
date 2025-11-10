"""
Two class to analyze FEBV2 commissioning tests
"""
import json
import picmic_register_access as pra
import ROOT
import numpy as np
from scipy import stats
class pedcor:
    """
    Class to analyze DAC10 bit scan of PETIROC and calculate DAC6bit correction per channel and DAC10bits threshold per ASIC
    """
    def __init__(self,state,version,feb,analysis,runid=None):
        """ Initialise the analysis and download a test result

        Args:
            state(str): Setup state used for the test
            version(int): Setup state version
            feb(int): Feb id of the test
            analysis(str): Name of the analysis SCURVE_1 or SCURVE_A
        """
        print(state,version)
        self.sdb=pra.instance()
        self.sdb.download_setup(state,version)
        self.scurves={}
        print(f"Getting Scurves  from analysis {analysis} on {state}/{version}")
        self.scurves=self.sdb.get_scurve(state,version,feb,analysis,runid=runid)
        self.full_done=False
    def draw_all(self,save=False,debug=False):
        """ Draw all Scurves of the test in ROOT TCanvas

        Args:
            save(bool): Save PDF (False by default)
            debug(bool): Draw Scurves one by one (False by default)
        """

        self.draw_scurves(self.scurves,save,debug)
    def draw_asic(self,title=None):
        """ Draw all Scurves of the test in ROOT TCanvas

        Args:
            save(bool): Save PDF (False by default)
            debug(bool): Draw Scurves one by one (False by default)
        """
        self.draw_one_asic(self.scurves,title=title)

    def analyse_scurve(self,ch,vals,thi,tha,fn,asic):
        nval=len(vals)
        print(ch)
        print(vals)
        if (ch<3):
            cc=input()
        # find maximum of scurve
        vmax=max(vals)
        indexes = [i for i, x in enumerate(vals) if x == vmax]
        imax=indexes[len(indexes)-1]
        # find first 0 of scurve after the max
        imin=imax
        for ith in range(imax,nval):
            if (vals[ith]<1):
                imin=ith
                break
        # Find maximum and minimum of diff
        vdiff=[0 for _ in range(nval+1)]
        for i in range(1,nval):
            vdiff[i] =vals[i-1]-vals[i]
        
        vdmax=max(vdiff)
        indexes = [i for i, x in enumerate(vdiff) if x == vdmax]
        idmax=indexes[len(indexes)-1]
        idmin=idmax
        for ith in range(idmax,nval):
            if ith<0:
                continue
            if ith>nval-1:
                continue
            print(ith,nval,len(vdiff))
            if (vdiff[ith]<1):
                idmin=ith
                break
        
        print(f"Channel {ch} Max {imax} to {imin} delta Max {idmax} to {idmin}")
        #cc=input()
        #histograms of the Scurve and of the differences
        shd="%s_%s_diff-c%d" % (fn,asic,ch)
        shs="%s_%s_scurve-c%d" % (fn,asic,ch)
        hd=ROOT.TH1F(shd,shd,tha-thi+1,thi,tha)
        hs=ROOT.TH1F(shs,shs,tha-thi+1,thi,tha)
        #diff=[0 for x in range(nval)]
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

        return hs,hd,imax,imin,ithmax,ithmin,vmax

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
        thrs=[0 for _ in range(64)]
        thre=[0 for _ in range(64)]
        thrm=[]
        thrr=[]
        v_thmax=[]
        v_thmin=[]
        all_lines=[]
        scfit=ROOT.TF1("scfit","[0]*TMath::Erfc((x-[1])/[2])",thi+1,tha);
        ROOT.gStyle.SetOptStat(0)
        hpmean=ROOT.TH1F("hpmean",f"Summary pedestal {fn} {asic} ",64,0.,64.0)
        hpnoise=ROOT.TH1F("hpnoise",f"Summary Noise {fn} {asic} ",64,0.,64.0)
        hpmean.GetYaxis().SetRangeUser(thi,tha)
        c1.Divide(1,3)
        c1.cd(1)
        for c in d_sc["channels"]:
            ch=c["prc"]
            vals=c["scurve"]
            hs,hd,imax,imin,ithmax,ithmin,vmax =self.analyse_scurve(ch,vals,thi,tha,fn,asic)

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
            #hsc.Fit("scfit","","");
            #thrs.append(scfit.GetParameter(1))
            #thre.append(scfit.GetParameter(2))
            thrs[ch]=thi+imax+(imin-imax)/2.
            thre[ch]=(imin-imax)/8.
            
            thrm.append(hd.GetMean())
            thrr.append(hd.GetRMS())
            v_thmax.append(ithmax)
            v_thmin.append(ithmin)
            c2.Draw()
            c2.Update()
            #print(f"Pedestal {hd.GetMean()} Noise {hd.GetRMS()}")
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
            """
            # Bad gaussian take 0.2-0.8 width
            if (thrr[i]>(v_thmin[i]-v_thmax[i]+1)):
                thrr[i]=(v_thmin[i]-v_thmax[i]+1)
            print(f"channel {i} ++Erf Fit++ {thrs[i]:.1f} Noise {thre[i]:.1f} ++ Gaussian ++ {thrm[i]:.1f} {thrr[i]:.1f} ++ 80 % ++ {v_thmax[i]:.1f} {v_thmin[i]:.1f} ")
            # Bad fit take the gaussian mean and RMS
            if (thre[i]>3*thrr[i]):
                thrs[i]=thrm[i]
                thre[i]=thrr[i]
            thr=thrs[i]+5*thre[i]
            """
            print(f"channel {i}  ++ {thrs[i]:.1f} {thre[i]:.1f} ")

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
            tl=ROOT.TLine(0,seuil,64,seuil)
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
            c1.SaveAs(f"results/{fn}_{asic}.png")
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
    def find_thresholds(self):
        """ Calculate turn on for one ASIC
        Args:
            asic(str): Asic name (LEFT_BOT...RIGHT_TOP)
        Returns:
            res an array of channels tuple (ch,turnon,crrection,mean asic turnon)
        """
        d_sc=self.scurves
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
            cor=(thop-th)/2
            #print(ch,th,round(cor),thop)
            print ("{:<8} {:<10} {:<10} {:<10}".format(ch,th,round(cor),thop))
            res.append((ch,th,round(cor),thop))
            #print(f"sdb.correct_6bdac({ch},{round(cor)},{self.asic})")
        
        return res
    def draw_one_asic(self,d_sc,title):
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
        c2.SaveAs(f"results/{fn}_{asic}.png")
        c2.SaveAs(f"results/{fn}_{asic}.pdf")
        v=input()
        return histos

       
