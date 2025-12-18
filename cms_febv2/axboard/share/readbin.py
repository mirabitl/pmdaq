#!/usr/bin/env python3


import sys, os
import logging
import time
import csv
import matplotlib.pyplot as plt
import ax_storage as ps
import os
import json
import  cms_irpc_feb_lightdaq as daq
from cms_irpc_feb_lightdaq.tools import *
from cms_irpc_feb_lightdaq.feb import FEB_ID, FEB_NAME, FPGA_ID, FPGA_NAME, ASIC_ID, ASIC_NAME
from cms_irpc_feb_lightdaq.AX7325BBoard import TdcUplinkFrame,TdcData

import numpy as np
import ROOT as R
c1=R.TCanvas()
t=TdcUplinkFrame([])
def rh_handler(psi):
    psi.logger.warning(f"Run {psi.run} {psi.runheader}")
    print(psi.new_run_header)
    input("New run header found , hit return")
hd =[None for _ in range(64)]
def ev_handler(psi):
    psi.logger.info(f"{psi.event} at {psi.date}")
    psi.logger.info(f"{psi.event} Number of blocks {len(psi.words)}")
    for p in range(len(psi.words)):
        psi.logger.info(f"Block {p} Number of  words {len(psi.words[p])} at {p}")
        tdcframes = []
        restored_payload = np.frombuffer(psi.words[p], dtype=np.uint64)
        for frame_32 in batched_it(restored_payload, 8):
            frame_32u = list(frame_32)
            frame_32=[int(i) for i in frame_32u]
            #print(frame_32)
            frame_256 = ((frame_32[7]<<224) | 
                         (frame_32[6]<<192) |
                         (frame_32[5]<<160) |
                         (frame_32[4]<<128) |
                         (frame_32[3]<<96)  | 
                         (frame_32[2]<<64)  |
                         (frame_32[1]<<32)  |
                          frame_32[0])
            #frame_256 = b''.join([int(i).to_bytes(4, byteorder='little') for i in frame_32])
            #print(f"frames 256 {frame_256:x} ")
            #input()
            #frame_256=int.from_bytes(frame_256, byteorder='big')
            tdcframes.append(TdcUplinkFrame(frame_256))
        tdcdata = {
            'FEB0' : {
                'LEFT'   : [],
                'MIDDLE' : [],
                'RIGHT'  : []
            },
            'FEB1' : {
                'LEFT'   : [],
                'MIDDLE' : [],
                'RIGHT'  : []
            }
        }
        psi.logger.info(f"{psi.event} Number of frames found {len(tdcframes)}")
        for frame in tdcframes:
            if frame.feb0_scframe or frame.feb1_scframe:
                psi.logger.error("Not TDC frame")
                continue
            #print(frame)
            #input()
            if frame.feb0_dvalid_0:
                fpga_id = frame.feb0_devaddr_0
                diff = frame.feb0_diff_0 if frame.feb0_strip_0 else None
                tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_0, val=frame.feb0_tdc_data_0, diff=diff, bc0id=frame.bc0id))

            if frame.feb0_dvalid_1:
                fpga_id = frame.feb0_devaddr_1
                diff = frame.feb0_diff_1 if frame.feb0_strip_1 else None
                tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_1, val=frame.feb0_tdc_data_1, diff=diff, bc0id=frame.bc0id))

            if frame.feb0_dvalid_2:
                fpga_id = frame.feb0_devaddr_2
                tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_2, val=frame.feb0_tdc_data_2, diff=None, bc0id=frame.bc0id))

            if frame.feb1_dvalid_0:
                fpga_id = frame.feb1_devaddr_0
                diff = frame.feb1_diff_0 if frame.feb1_strip_0 else None
                tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_0, val=frame.feb1_tdc_data_0, diff=diff, bc0id=frame.bc0id))

            if frame.feb1_dvalid_1:
                fpga_id = frame.feb1_devaddr_1
                diff = frame.feb1_diff_1 if frame.feb1_strip_1 else None
                tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_1, val=frame.feb1_tdc_data_1, diff=diff, bc0id=frame.bc0id))

            if frame.feb1_dvalid_2:
                fpga_id = frame.feb1_devaddr_2
                tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_2, val=frame.feb1_tdc_data_2, diff=None, bc0id=frame.bc0id))
        for fpga in daq.FPGA_ID:
            chan_ts = [[] for _ in range(34)]
            for chan, ts, diff,*_ in tdcdata['FEB0'][fpga]:
                chan_ts[chan].append(ts)
            t0=chan_ts[33][0]
            tmin = -965.
            tmax = -910.;
            for ch in range(34):
                if (len(chan_ts[ch])==0):
                    continue
                if (ch<32):
                    for t in chan_ts[ch]:
                        d = t-t0
                        print(t,t0,d)
                        if d>tmin and d<tmax:
                            print(f"Found {fpga} {ch} {t} {d}")
                            input("alors")
                mu = np.mean(chan_ts[ch])
                sigma = np.std(chan_ts[ch])
                print(f"FPGA {fpga} TDC channel {ch}: {len(chan_ts[ch])} timestamps, mean={mu:.8}, std={sigma:.3}")


    input("New Event found  , hit return")
    return
    ts = [[] for _ in range(64)]
    diff = []
    first = [True]*64
    first_sample=True
    newwin=False
    t0=0
    for p in range(len(psi.words)):
        psi.logger.debug(f" words {len(psi.words[p])} at {p}")
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
                    #print(ch,coarse,fine,(coarse<<13 | fine)*3.0523E-3)
                    ts[ch].append((coarse<<13 | fine)*3.0523E-3)
                    t0=(coarse<<13 | fine)*3.0523E-3
                    newwin=True
                else:
                    #print(ch,t0,(coarse<<13 | fine)*3.0523E-3)
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
                psi.logger.debug(f"{psi.event} ch{lch} nbhit={len(ts[pch])}, mean={np.mean(ts[pch]):.8}, std={np.std(ts[pch]):.3}")
                if (len(ts[pch])==len(ts[0])):
                    td= np.subtract(ts[pch],ts[0]).tolist()
                    mch=np.mean(td)
                    rch=np.std(td)
                    psi.logger.debug(f"Substracted ch{lch} nbhit={len(td)}, mean={mch:.8}, std={rch:.3}")
                    ts[pch]=td
                    if (psi.new_run_header):
                        if hd[pch] == None:
                            hd[pch]=R.TH1F(f"chanP{pch}L{lch}",f"chanP{pch}L{lch}",500,mch-20*rch,mch+20*rch)
                        else:
                            c1.cd()
                            hd[pch].Draw()
                            hd[pch].Fit("gaus")
                            c1.Update()
                            c1.Draw()
                            input()
                            hd[pch].Reset()
                    for x in td:
                        if hd[pch] !=None:
                            hd[pch].Fill(x)
                else:
                     psi.logger.error(f"Wromg length {len(ts[pch])}")
                    #input()
        else:
            psi.logger.debug(f"ch{lch} nbhit={len(ts[lch])}, mean={np.mean(ts[lch]):.8}, std={np.std(ts[lch]):.3}") 
    psi.new_run_header=False

psr=ps.storage_manager()
psr.run_handler=rh_handler
psr.event_handler=ev_handler

psr.read(sys.argv[1]) #"/home/acqilc/unessai.bin")
