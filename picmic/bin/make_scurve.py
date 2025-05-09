import logging
import time
import matplotlib.pyplot as plt

import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
import liroc_ptdc_daq as daq





map_liroc_to_ptdc_chan = {
    62: 3 ,
    58: 4 ,
    63: 5 ,
    54: 6 ,
    60: 7 ,
    61: 8 ,
    56: 9 ,
    44: 10,
    50: 11,
    59: 12,
    46: 13,
    57: 14,
    52: 15,
    48: 16,
    53: 17,
    51: 18,
    42: 19,
    49: 20,
    40: 21,
    47: 22,
    38: 23,
    45: 24,
    36: 25,
    43: 26,
    34: 27,
    41: 28,
    32: 29,
    39: 30,
    30: 31,
    37: 32,
    28: 33,
    35: 34,
    26: 35,
    33: 36,
    24: 37,
    31: 38,
    22: 39,
    29: 40,
    20: 41,
    27: 42,
    18: 43,
    25: 44,
    16: 45,
    23: 46,
    21: 47,
    12: 48,
    19: 51,
    5 : 50,
    17: 49,
    6 : 52,
    15: 53,
    4 : 54,
    13: 55,
    14: 56,
    11: 57,
    10: 58,
    2 : 59,
    3 : 60,
    7 : 61,
    8 : 62,
    1 : 63,
}



    
if __name__ == '__main__':
    daq.configLogger(logging.DEBUG)
    
    kc705 = daq.KC705Board()
    kc705.init()
    
    feb = daq.FebBoard(kc705)
    feb.init()
    
    
    feb.loadConfigFromCsv(folder=f".", config_name='good_config.csv')
    feb.fpga.enableDownlinkFastControl()
    # disable all liroc channels
    for ch in range(64): feb.liroc.maskChannel(ch)
    feb.ptdc.powerup()
    
    kc705.fastbitConfigure(mode='normal', flush_delay=100,
        dig0_edge='rising', dig0_delay=0, 
        dig1_edge='rising', dig1_delay=0)
        


    print("Liroc channel; Ptdc channel; scurve start; scurve stop; scurve step; scurve values")

    lmmcx_front=[1,2,3,4,5,6,7,8,14,30,25,45,38,46,48,50]
    lmmcx_spare=[55,59,53,54,57,61,52,56,63,58,62,60,50]
    lerni_a=[9,12,10,17,18,21,23,27,29,28,33,34,36,39,41,44,50]
    lerni_b=[11,13,15,16,19,20,22,24,26,31,32,35,37,40,42,43,50]
    lchan =lmmcx_front[:4]
    lchan=range(64)
    for lichan in lchan:
        if lichan not in map_liroc_to_ptdc_chan: continue
        #if lichan == 51: continue
    
        # helper functions
        def get_plateau_limits(scurve):
            idx_limit_high = 0
            idx_limit_low = len(scurve)-1
            # flags to have the outermost plateau regions if several transition
            stop_high = False
            stop_low = False
            for i in range(len(scurve)):
                if scurve[i] > scurve[0]*0.9 and (not stop_high):
                    idx_limit_high = i
                else:
                    stop_high = True

                if (scurve[len(scurve)-1-i] < scurve[0]*0.1) and (not stop_low):
                    idx_limit_low = len(scurve)-1-i
                else:
                    stop_low = True

            return idx_limit_high, idx_limit_low
            
        def sweep_dac10b(*args):
            scurves = [[] for _ in range(64)]
            for dac10b_val in range(*args):
                feb.liroc.set10bDac(dac10b_val)
                feb.liroc.stopScClock()
                #print(dac10b_val)
                #time.sleep(0.1)
                counters = kc705.acqPtdcCounters(window_ms=0.1)
                for ch, counter in enumerate(counters):
                    scurves[ch].append(counter)
            return scurves


        def scurve_single_chan(lichan):
            for ch in range(64):
                feb.liroc.setChannelDac(ch, 32)
                feb.liroc.maskChannel(ch, ch != lichan)
                feb.liroc.setParam(f'DC_PA_ch{ch}',2)
            start, stop, step = 450, 750,1
            scurve = sweep_dac10b(start, stop, step)[map_liroc_to_ptdc_chan[lichan]]
            return scurve, start, stop, step


        scurve, start, stop, step = scurve_single_chan(lichan)
        # keep only transition and 10 points before and after if possible
        # idx_limit_high__, idx_limit_low__ = get_plateau_limits(scurve)
        # idx_start = idx_limit_high__ - 10 if idx_limit_high__ >= 10 else 0
        # idx_stop = idx_limit_low__ + 10 if idx_limit_low__ + 10 < len(scurve) else len(scurve)-1
        # print(f"{lichan}; {map_liroc_to_ptdc_chan[lichan]}; {start+idx_start}; {start+idx_stop}; {step}; {scurve[idx_start:idx_stop]}")
        
        print(f"{lichan}; {map_liroc_to_ptdc_chan[lichan]}; {start}; {stop}; {step}; {scurve}")
        plt.plot(range(start, stop, step), scurve, '+-')
    plt.show()

