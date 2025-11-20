#!/usr/bin/env python3


import sys, os
import logging
import time
import csv
import matplotlib.pyplot as plt
import picmic_storage as ps
import os
import json
import liroc_ptdc_daq as daq
import numpy as np
import ROOT as R
c1=R.TCanvas()
def rh_handler(psi):
    psi.logger.warning(f"Run {psi.run} {psi.runheader}")
    print(psi.new_run_header)

hd =[None for _ in range(64)]
def ev_handler(psi):
    psi.logger.info(f"{psi.event} at {psi.date}")
    psi.logger.info(f"{psi.event} Number of blocks {len(psi.words)}")
    ts = [[] for _ in range(64)]
    diff = []
    first = [True]*64
    first_sample=True
    newwin=False
    t0=0
    for p in range(len(psi.words)):
        psi.logger.info(f" words {len(psi.words[p])} at {p}")
        for word in psi.words[p]:
            word   = daq.BitField(word)
        
            if word[31] == 1:
                assert word[30,28] == 0b000, f"{word:08X}"
            
                bxcount     = word[27,15]
                coarsecount = word[14,2]
                #print(f"{bxcount=} {coarsecount=}")
            
            else:
                ch     = word[30,27] + 16*p
                if first[ch]:
                    first[ch] = False
                    continue
                edge   = word[26]
                coarse = word[25, 13]
                fine   = word[12,0]
                if (ch==0):
                    print(ch,coarse,fine,(coarse<<13 | fine)*3.0523E-3)
                    ts[ch].append((coarse<<13 | fine)*3.0523E-3)
                    t0=(coarse<<13 | fine)*3.0523E-3
                    newwin=True
                else:
                    print(ch,t0,(coarse<<13 | fine)*3.0523E-3)
                    if (first_sample):
                        if (newwin): # Keep only first signal in ACQ
                            ts[ch].append(((coarse<<13 | fine)*3.0523E-3))
                            newwin=False
                    else:
                        ts[ch].append(((coarse<<13 | fine)*3.0523E-3))

    #print('')
    tdiff=[]

    for lch in range(64):
        if lch not in daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN and lch!=0: continue
        if (lch!=0):
            pch=daq.FebBoard.MAP_LIROC_TO_PTDC_CHAN[lch]
            if (len(ts[pch])>0):
                psi.logger.info(f"{psi.event} ch{lch} nbhit={len(ts[pch])}, mean={np.mean(ts[pch]):.8}, std={np.std(ts[pch]):.3}")
                if (len(ts[pch])==len(ts[0])):
                    td= np.subtract(ts[pch],ts[0]).tolist()
                    mch=np.mean(td)
                    rch=np.std(td)
                    psi.logger.info(f"Substracted ch{lch} nbhit={len(td)}, mean={mch:.8}, std={rch:.3}")
                    ts[pch]=td
                    if (psi.new_run_header):
                        if hd[pch] == None:
                            hd[pch]=R.TH1F(f"chan{pch}",f"chan{pch}",500,mch-20*rch,mch+20*rch)
                        else:
                            c1.cd()
                            hd[pch].Draw()
                            hd[pch].Fit("gaus")
                            c1.Update()
                            c1.Draw()
                            input()
                            hd[pch].Reset()
                    for x in td:
                        hd[pch].Fill(x)
                else:
                     psi.logger.error(f"Wromg length {len(ts[pch])}")
                    #input()
        else:
            psi.logger.info(f"ch{lch} nbhit={len(ts[lch])}, mean={np.mean(ts[lch]):.8}, std={np.std(ts[lch]):.3}") 
    psi.new_run_header=False

psr=ps.storage_manager()
psr.run_handler=rh_handler
psr.event_handler=ev_handler

psr.read("/home/acqilc/unessai.bin")
