import ROOT as R
from array import array

c1=R.TCanvas()

def makegraph(x,y,dx,dy,title,xlabel=None,ylabel=None,ymin=None,ymax=None,xmin=None,xmax=None):
    gr4 = R.TGraphErrors( len(x), x,y,dx,dy)

    gr4.SetMarkerColor( 2 )
    gr4.SetMarkerStyle( 21 )
    gr4.SetTitle( title)
    if (xlabel!=None):
        gr4.GetXaxis().SetTitle(xlabel)
    if (ylabel!=None):
        gr4.GetYaxis().SetTitle(ylabel)

    c1.cd()
    if (ymin!=None):
        gr4.GetYaxis().SetRangeUser(ymin,ymax)
    if (xmin!=None):
        gr4.GetXaxis().SetRangeUser(xmin,xmax)
    gr4.Draw( 'AP' )
    c1.Draw()
    c1.Update()
    #v=input()
    return gr4

def plot_mean(fname,ch,vmin,vmax,vstep,rise,threshold):
    xinj=array( 'd' )
    dxinj=array('d')
    x_mean=array( 'd' )
    dx_mean=array( 'd' )
    x_rms=array( 'd' )
    dx_rms=array( 'd' )

    _file0=R.TFile.Open(fname)
    for v in range(vmin,vmax,vstep):
        hd=_file0.Get(f"diff{ch}_{v}")
        if not hd:
            continue
        print(f"Injection {v*11./32.} fC {hd.GetMean():.2f} {hd.GetRMS()*1000:.2f}")
        c1.cd()
        hd.Fit("gaus")
        c1.Draw()
        c1.Update()
        #input()
        xinj.append(v*11./32.)
        dxinj.append(1*11/32)
        x_mean.append(hd.GetMean())
        dx_mean.append(hd.GetMeanError())
        x_rms.append(hd.GetRMS()*1000)
        dx_rms.append(hd.GetRMSError()*1000)
    gm= makegraph(xinj,x_mean,dxinj,dx_mean,f"Time walk vs injection {rise} ns DAC {threshold-512}",xlabel="Charge injection (fC)",ylabel="Delay (ns)",ymin=None,ymax=None)
    c1.SaveAs(f"Mean_vs_charge_R{rise}ns_T{threshold-512}.pdf")
    gr= makegraph(xinj,x_rms,dxinj,dx_rms,f"Resolution vs injection {rise} ns DAC {threshold-512}",xlabel="Charge injection (fC)",ylabel="RMS (ps)",ymin=None,ymax=None)
    c1.SaveAs(f"Resolution_vs_charge_R{rise}ns_T{threshold-512}.pdf")

import json
import math
def plot_summary(nrun):
    xinj=array( 'd' )
    dxinj=array('d')
    xinj0=array( 'd' )
    dxinj0=array('d')
    x_mean=array( 'd' )
    dx_mean=array( 'd' )
    x_rms=array( 'd' )
    dx_rms=array( 'd' )
    x_eps=array( 'd' )
    dx_eps=array( 'd' )

    s=json.loads(open(f'results{nrun}.json').read())
    sc=s["def"]["configuration_list"][s["def"]["configuration"]]
    vmin=int(sc["vmin"]*1000)
    vmax=int(sc["vmax"]*1000)
    vstep=int((vmax-vmin)/sc["nstep"])
    for v in range(vmin,vmax,vstep):
        if not f'{v}' in s["stat"].keys():
            continue
        xinj0.append(v*11./32)
        dxinj0.append(11./32);
        
        re=s["stat"][f'{v}']
        eff=re["nseen"]/re["ntot"]
        if eff<0.999:
            deff=math.sqrt(eff*(1-eff)/re["ntot"])
        else:
            deff=math.sqrt(1./re["ntot"])
        x_eps.append(eff*100)
        dx_eps.append(deff*100)
        if eff<0.95:
            continue

        if not f'{v}' in s["channels"]["CH13"].keys():
            continue
        r=s["channels"]["CH13"][f'{v}']
        xinj.append(v*11./32)
        dxinj.append(11./32);
        dxinj.append(1*11/32)
        x_mean.append(r['mean'])
        dx_mean.append(r['d_mean'])
        x_rms.append(r['rms'])
        dx_rms.append(r['d_rms'])
       
        
    threshold=sc["threshold"]
    rise=sc['rise']
    gm= makegraph(xinj,x_mean,dxinj,dx_mean,f"Time walk vs injection {rise} ns DAC {threshold-512}",xlabel="Charge injection (fC)",ylabel="Delay (ns)",ymin=None,ymax=None)
    c1.SaveAs(f"Mean_vs_charge_{nrun}_R{rise}ns_T{threshold-512}.pdf")
    gr= makegraph(xinj,x_rms,dxinj,dx_rms,f"Resolution vs injection {rise} ns DAC {threshold-512}",xlabel="Charge injection (fC)",ylabel="RMS (ps)",ymin=None,ymax=None)
    c1.SaveAs(f"Resolution_vs_charge_{nrun}_R{rise}ns_T{threshold-512}.pdf")
    ge= makegraph(xinj0,x_eps,dxinj0,dx_eps,f"Efficiency vs injection {rise} ns DAC {threshold-512}",xlabel="Efficiency (%)",ylabel="RMS (ps)",ymin=None,ymax=None,xmin=0,xmax=300)
    c1.SaveAs(f"Efficiency_vs_charge__{nrun}R{rise}ns_T{threshold-512}.pdf")
    
    # time walk
    maxtime=-1.
    mintime=1E9
    imatime=0
    imitime=0
    for i in range(len(xinj)):
        if x_mean[i]<mintime:
            mintime=x_mean[i]
            imitime=i
        if x_mean[i]>maxtime:
            maxtime=x_mean[i]
            imatime=i
    timewalk=maxtime-mintime
    #print(f"{xinj[imitime]:.2f} fC {mintime:.2f} ns {xinj[imatime]:.2f} fC {maxtime:.2f}  ns Time walk {maxtime-mintime:.2f} ns") 
    
    #res
    maxres=-1.
    minres=1E9
    imares=0
    imires=0
    for i in range(len(xinj)):
        if x_rms[i]<minres:
            minres=x_rms[i]
            imires=i
        if x_rms[i]>maxres:
            maxres=x_rms[i]
            imares=i
    
    #print(f"{xinj[imires]:.2f} fC {minres:.2f} ps {xinj[imares]:.2f} fC {maxres:.2f}  ps ") 

    #eff and thr
    xmin=10000
    xmax=0
    for i in range(len(xinj0)):
        if x_eps[i]>0:
            xmin=xinj0[i]
            break
    for i in range(len(xinj0)-1,0,-1):
        if x_eps[i]<99:
            xmax=xinj0[i]
            break
    thr =(xmax+xmin)/2.
    #print(f'{xmin:.2f} {xmax:.2f} {(xmax+xmin)/2.:.2f} fC {threshold} DAC {(xmax+xmin)/2./(threshold-512)} fC/DAC ')
    #print(f'{nrun},{threshold:.2f},{thr:.2f},{timewalk:.2f},{xinj[imares]:.2f},{maxres:.2f},{xinj[imires]:.2f},{minres:.2f}')
    print(f'{nrun},{threshold:.2f},{thr:.2f},{timewalk:.2f},{xinj[imares]:.2f},{maxres:.2f},{xinj[imires]:.2f},{minres:.2f}')
