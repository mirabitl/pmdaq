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
from collections import namedtuple

# Définition du namedtuple
TdcChannel = namedtuple('TdcChannel', ['chan', 'raw', 'diff', 'bc0id','time','strip','side'])

c1=R.TCanvas()


class febEvent:
    def __init__(self):
        self.run_header=None
        self.nread=0
        self.tdcframes=[]
        self.tmax = -785.0
        self.tmin = -825.;0
                #tmin = -855;t
        self.htm=[R.TH1F(f"dt{lch}",f"dt{lch}",50,self.tmin-5,self.tmax+5) for lch in range(32)]
        self.html=[R.TH1F(f"dtl{lch}",f"dtl{lch}",3000,-300000.,0.) for lch in range(32)]
        self.nfound=0
        self.tdcdata = {
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
        s=json.loads(open("/home/acqilc/rocanalysis/lmana/etc/dome_mar2025/FEBV2_R3_1.json").read())["content"]

        self.mf={"LEFT":[None for _ in range(32)],"MIDDLE":[None for _ in range(32)],"RIGHT":[None for _ in range(32)]}
        for x in s.keys():
            y=s[x]
            fp=y[0].upper()
            sn=x.split("_")[0]
            ns=int(x.split("_")[1])
            side=0
            if sn=="strip":
                side=1
            ch=y[3]
            pch=y[2]
            self.mf[fp][ch]=(ch,pch,side,ns,x)
        print(self.mf)
        input()
    def clear(self):
        self.tdcframes.clear()
        for feb in ['FEB0','FEB1']:
            for fp in ['LEFT','MIDDLE','RIGHT']:
                self.tdcdata[feb][fp].clear()
    def rh_handler(self,psi):
        psi.logger.warning(f"Run {psi.run} {psi.runheader}")
        print(psi.new_run_header)
        input("New run header found , hit return")
    def ev_handler(self,psi):
        self.clear()
        self.nread+=1
        if psi.event%100==0:
            psi.logger.info(f"===============> {psi.event} at {psi.date}")
        psi.logger.debug(f"{psi.event} Number of blocks {len(psi.words)}")
        for p in range(len(psi.words)):
            psi.logger.debug(f"Block {p} Number of  words {len(psi.words[p])} at {p}")
            self.tdcframes = []
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
                self.tdcframes.append(TdcUplinkFrame(frame_256))
            
            psi.logger.debug(f"{psi.event} Number of frames found {len(self.tdcframes)}")
            for frame in self.tdcframes:
                if frame.feb0_scframe or frame.feb1_scframe:
                    psi.logger.error("Not TDC frame")
                    continue
                #print(frame)
                #input()
                if frame.feb0_dvalid_0:
                    fpga_id = frame.feb0_devaddr_0
                    diff = frame.feb0_diff_0 if frame.feb0_strip_0 else None
                    self.tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_0, val=frame.feb0_tdc_data_0, diff=diff, bc0id=frame.bc0id))

                if frame.feb0_dvalid_1:
                    fpga_id = frame.feb0_devaddr_1
                    diff = frame.feb0_diff_1 if frame.feb0_strip_1 else None
                    self.tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_1, val=frame.feb0_tdc_data_1, diff=diff, bc0id=frame.bc0id))

                if frame.feb0_dvalid_2:
                    fpga_id = frame.feb0_devaddr_2
                    self.tdcdata['FEB0'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb0_chanid_2, val=frame.feb0_tdc_data_2, diff=None, bc0id=frame.bc0id))

                if frame.feb1_dvalid_0:
                    fpga_id = frame.feb1_devaddr_0
                    diff = frame.feb1_diff_0 if frame.feb1_strip_0 else None
                    self.tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_0, val=frame.feb1_tdc_data_0, diff=diff, bc0id=frame.bc0id))

                if frame.feb1_dvalid_1:
                    fpga_id = frame.feb1_devaddr_1
                    diff = frame.feb1_diff_1 if frame.feb1_strip_1 else None
                    self.tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_1, val=frame.feb1_tdc_data_1, diff=diff, bc0id=frame.bc0id))

                if frame.feb1_dvalid_2:
                    fpga_id = frame.feb1_devaddr_2
                    self.tdcdata['FEB1'][FPGA_NAME[fpga_id]].append(TdcData(chan=frame.feb1_chanid_2, val=frame.feb1_tdc_data_2, diff=None, bc0id=frame.bc0id))
            ORB_LEN=92175.00
            found=False
            for fpga in daq.FPGA_ID:
                self.chan_ts = [[] for _ in range(34)]
                for chan, ts, diff,_bc0_id in self.tdcdata['FEB0'][fpga]:
                    #self.chan_ts[chan].append(ts)
                    tc=(_bc0_id-1)*ORB_LEN+ts*2.5/256
                    self.chan_ts[chan].append(tc)
                if (len(self.chan_ts[33])==0):
                    continue
                t0=self.chan_ts[33][0]
                
                #tmin = -855;tmax = -805
                for ch in range(34):
                    if (len(self.chan_ts[ch])==0):
                        continue
                    if (ch<32):
                        for t in self.chan_ts[ch]:
                            d = t-t0
                            #print(t,t0,d,tmin,tmax)
                            if fpga=="MIDDLE":
                                self.html[ch].Fill(d)
                            if d>self.tmin and d<self.tmax:
                                self.htm[ch].Fill(d)
                                psi.logger.debug(f"Found {fpga} {ch} {t} {d}")
                                found=True
                    mu = np.mean(self.chan_ts[ch])
                    sigma = np.std(self.chan_ts[ch])
                    psi.logger.debug(f"FPGA {fpga} TDC channel {ch}: {len(self.chan_ts[ch])} timestamps, mean={mu:.8}, std={sigma:.3}")

        if found:
            self.nfound=self.nfound+1
        psi.logger.info(f"New Event {psi.event} found  In Time {found} # {self.nfound} Eps  {self.nfound/(psi.event+1)*100:.2f}")
        return

    def end_handler(self):
        for ch in range(32):
            c1.cd()
            self.htm[ch].Draw()
            c1.Update()
            c1.Draw()
            input()
            c1.cd()
            self.html[ch].Draw()
            c1.Update()
            c1.Draw()
            input()

psr=ps.storage_manager()
evh=febEvent()
psr.run_handler=evh.rh_handler
psr.event_handler=evh.ev_handler
psr.end_handler=evh.end_handler

psr.read(sys.argv[1]) #"/home/acqilc/unessai.bin")
