#!/usr/bin/env python3

import numpy as np
from pprint import pprint
import datetime
import struct
import zstandard as zstd

import sys, os
import logging
import time
import json


class storage_manager:
    def __init__(self):
        self.data_file_name = "acq_compressed.bin"
        self.index_file_name = "acq_compressed.idx"
        self.compressor = zstd.ZstdCompressor(level=3)
        self.decompressor = zstd.ZstdDecompressor()

        self.offset=0
        self.event=0
        self.run=0
        self.run_handler=None
        self.event_handler=None
        self.logger=logging.getLogger(__name__)
        self.new_run_header=False
        logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    def open(self,fname):
        self.data_file_name = fname+".bin"
        self.index_file_name = fname+".idx"
        self.f_data=open(self.data_file_name, "wb")
        self.f_idx=open(self.index_file_name, "wb")
        self.event=0
    def writeRunHeader(self,run_number,payload):
        self.run=run_number
        self.logger.info(f"New run header {self.run} payload {payload} ")
        b_type=0
        raw_bytes =payload.tobytes()
        compressed = self.compressor.compress(raw_bytes)
        comp_size = len(compressed)
        self.f_data.write(struct.pack("<IIIQ",b_type,self.run,comp_size,int(time.time_ns())))
        self.f_data.write(compressed)
        self.f_idx.write(struct.pack("<IQ", b_type,self.offset))
        self.offset+=3*4+8+comp_size
    def writeEvent(self,payloads):

        b_type=1
        e_offset=0
        self.f_data.write(struct.pack("<IIIQ",b_type,self.event,len(payloads),int(time.time_ns())))
        e_offset+=3*4+8
        idx=0
        self.logger.debug(f"Write {self.event} # Payloads {len(payloads)} ")
        for payload in payloads:
            self.logger.debug(len(payload))

            raw_bytes =np.array(payload).tobytes()
            compressed = self.compressor.compress(raw_bytes)
            comp_size = len(compressed)
            bl_type=(idx<<4)
            self.f_data.write(struct.pack("<IIII",bl_type,self.event,idx,comp_size))

            idx+=1
            self.f_data.write(compressed)
            e_offset+=4*4+comp_size
        self.f_idx.write(struct.pack("<IQ", b_type,self.offset))
        self.offset+=e_offset
        self.event+=1

    def close(self):
        self.f_data.close()
        self.f_idx.close()
        

    def read(self,fname):
        self.f_data=open(fname, "rb")
        while True:
            try:
                header=self.f_data.read(20)
                if (len(header)<20):
                    self.logger.warn(f"{len(header)} --->End of file reached")
                    break
            except Exception as e:
                self.logger.debug(f"{repr(e)} --->End of file reached")
                break
            b_type,number,length,timestamp=struct.unpack("<IIIQ", header)
            self.time=timestamp
            self.date=datetime.datetime.fromtimestamp(timestamp / 1e9)

            # run header
            if (b_type == 0):
                self.run = number
                compressed=self.f_data.read(length)
                raw_bytes = self.decompressor.decompress(compressed)
                self.runheader = np.frombuffer(raw_bytes, dtype=np.uint32).copy()
                self.logger.debug(f"new run header {self.run} : {self.runheader}")
                self.new_run_header=True
                # Call the run header handler
                if self.run_handler != None:
                    self.run_handler(self)
                continue
            # Event
            if (b_type==1):
                self.event=number
                self.logger.debug(f"\t new event {self.event} # blocks {length}")
                self.words=[ [] for _ in range(length)]
                
                for p in range(length):
                    bl_header=self.f_data.read(16)
                    bl_type,bl_event,id_block,comp_size=struct.unpack("<IIII",bl_header)
                    compressed = self.f_data.read(comp_size)
                    raw_bytes = self.decompressor.decompress(compressed)
                    self.words[p] = np.frombuffer(raw_bytes, dtype=np.uint64).copy()
                    self.logger.debug(f"\t \t {p} blocks {id_block} for event {bl_event} # block size {comp_size}")
                # Call the event handler
                if self.event_handler != None:
                    self.event_handler(self)
