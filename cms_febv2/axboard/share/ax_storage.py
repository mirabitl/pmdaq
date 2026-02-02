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
import base64

from typing import Union
from array import array

_PM_HEADER_FMT = "<IIIQI"  # PMDAQ header

def store(detid: int,
          sourceid: int,
          eventid: int,
          bxid: int,
          payload: Union[bytes, bytearray, memoryview],
          destdir: str,
          *,
          add_header: bool = True,
          add_readout_markers: bool = False) -> None:
    """
    Écriture PMDAQ avec marqueurs READOUT hors payload.

    Layout si add_readout_markers=True :
        '(' + [header] + payload + ')'

    Si add_header=False :
        '(' + payload + ')'
    """

    payload = bytes(payload)

    chunks = []

    # --- marqueur de début ---
    if add_readout_markers:
        chunks.append(READOUT_START)

    # --- header PMDAQ ---
    if add_header:
        header = struct.pack(
            _PM_HEADER_FMT,
            detid,
            sourceid,
            eventid,
            bxid,
            len(payload)
        )
        chunks.append(header)

    # --- payload brut ---
    chunks.append(payload)

    # --- marqueur de fin ---
    if add_readout_markers:
        chunks.append(READOUT_END)

    to_write = b"".join(chunks)

    # --- écriture du fichier ---
    filename = os.path.join(
        destdir,
        f"Event_{detid}_{sourceid}_{eventid}_{bxid}"
    )

    try:
        with open(filename, "wb") as f:
            written = f.write(to_write)
            if written != len(to_write):
                print(f"pb in write {written}")
                return
    except OSError as e:
        print(f"No way to store to file: {e}")
        return

    # --- fichier closed/ ---
    closed_dir = os.path.join(destdir, "closed")
    os.makedirs(closed_dir, exist_ok=True)

    closed_filename = os.path.join(
        closed_dir,
        f"Event_{detid}_{sourceid}_{eventid}_{bxid}"
    )

    with open(closed_filename, "a"):
        os.utime(closed_filename, None)


class PyFebWriter:
    def __init__(self,shm_directory="/dev/shm/feb_data/"):
        self._event = 0
        self._eventSize = 0   # nombre de uint32
        self._buffer = bytearray(1024 * 1024)  # taille arbitraire
        self._idx = 0
        self.shm_directory=shm_directory

    def setIds(self,detid,sourceid):
        self.detector_id=detid
        self.source_id=sourceid
    def newEvent(self):
        self._event += 1
        self._eventSize = 0
        self._idx = 0

        # READOUT_START
        self._buffer[0] = READOUT_START

        # écrire event (uint32) à offset 1
        self._buffer[1:5] = self._event.to_bytes(4, byteorder="little")

        # eventSize sera écrit plus tard à offset 5
        self._idx = 9

    def appendEventData(self, values):
        """
        values : iterable de uint32
        """
        arr = array('I', values)          # uint32 natif
        raw = arr.tobytes()

        nwords = len(values)
        self._eventSize += nwords

        self._buffer[self._idx:self._idx + len(raw)] = raw
        self._idx += len(raw)

        return self._idx

    def writeEvent(self):
        # écrire eventSize (uint32) à offset 5
        self._buffer[5:9] = self._eventSize.to_bytes(4, byteorder="little")

        # READOUT_END
        self._buffer[self._idx] = READOUT_END
        self._idx += 1

        # buffer final prêt pour utils::store
        store(detid=self.detector_id,sourceid=self.source_id,eventid=self._event,bxid=self._event,
        payload=bytes(self._buffer[:self._idx]),destdir=self.shm_directory,add_header=False,add_readout_markers=False)
        #return bytes(self._buffer[:self._idx])
        return


class storage_manager:
    def __init__(self,fdir='./'):
        self.fdir=fdir
        self.data_file_name = self.fdir+"acq_compressed.bin"
        self.index_file_name = self.fdir+"acq_compressed.idx"
        self.compressor = zstd.ZstdCompressor(level=3)
        self.decompressor = zstd.ZstdDecompressor()

        self.offset=0
        self.event=0
        self.run=0
        self.run_handler=None
        self.event_handler=None
        self.init_handler=None
        self.end_handler=None
        self.logger=logging.getLogger(__name__)
        self.new_run_header=False
        logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
    def open(self,fname):
        self.data_file_name = self.fdir+fname+".bin"
        self.index_file_name = self.fdir+fname+".idx"
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
    def writeRunHeaderDict(self,run_number,input_dict,usebase64=False):
        self.run=run_number
        self.logger.info(f"New run header {self.run} payload {input_dict}")
        b_type=2
        
        if usebase64:
            message = str(input_dict)
            ascii_message = message.encode('ascii')
            output_byte = base64.b64encode(ascii_message)
            
            compressed = self.compressor.compress(output_byte)
            comp_size = len(compressed)
            self.f_data.write(struct.pack("<IIIQ",b_type,self.run,comp_size,int(time.time_ns())))
            self.f_data.write(compressed)
            self.f_idx.write(struct.pack("<IQ", b_type,self.offset))
            self.offset+=3*4+8+comp_size
        else:
            timestamp = int(time.time_ns())

            # JSON canonique, portable
            json_bytes = json.dumps(
                input_dict,
                separators=(",", ":"),  # compact
                sort_keys=True           # déterministe
            ).encode("utf-8")

            compressed = self.compressor.compress(json_bytes)
            comp_size = len(compressed)

            # Header binaire
            self.f_data.write(
                struct.pack("<IIIQ", b_type, self.run, comp_size, timestamp)
            )
            self.f_data.write(compressed)
            
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
        

    def read(self,fname,usebase64=False):
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
            if (b_type == 2):
                self.run = number
                if usebase64:
                    compressed=self.f_data.read(length)
                    raw_bytes = self.decompressor.decompress(compressed)
                    msg_bytes = base64.b64decode(raw_bytes)
                    ascii_msg = msg_bytes.decode('ascii')
                    # Json library convert stirng dictionary to real dictionary type.
                    # Double quotes is standard format for json
                    ascii_msg = ascii_msg.replace("'", "\"")
                    self.runheader = json.loads(ascii_msg)
                    #self.runheader = np.frombuffer(raw_bytes, dtype=np.uint32).copy()
                else:
                    compressed = self.f_data.read(length)
                    if len(compressed) != length:
                        raise IOError("Payload incomplet")

                    # Décompression
                    json_bytes = self.decompressor.decompress(compressed)
                    print(json_bytes)
                    #input()
                    # Décodage JSON
                    self.runheader = json.loads(json_bytes.decode("utf-8"))
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
        if self.end_handler != None:
            self.end_handler()
