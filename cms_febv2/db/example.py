"""!
@file example.py

@brief Example of csv_register module usage
"""

import csv_register_access as cra

def create_a_state(name,febid,comment="un state avec les valuers par defaut"):
    """!
    Create a new state with one FEB anbd default parameters

    @param name Name of the state
    @param febid Id of FEBV2
    @param comment A comment

    @return The febv2_setup created
    """
    s=cra.instance()
    s.create_setup(name)
    
    f1=cra.febv2_registers(febid)
    f1.load_defaults()
    s.setup.add_febv2(f1)
    s.upload_changes(comment)
    return s.setup
def download_a_state(name,version):
    """!
    Download a state and write csv files to /dev/shm/feb_csv

    @param name Name of the state
    @param version version number
    @return The febv2_setup created
    """
    s=cra.instance()
    s.download_setup(name,version)
    s.to_csv_files()
    return s.setup
def set_pair_filtering_re31(fp):
    """!
    Set pair filtering (iRPC RE3.1) for one feb_fpga_+registers object

    @param fp the fpga register object

    """
    fp.set_pair_filtering_en(15,1,"LEFT")
    fp.set_pair_ts_diff_min(15,89,"LEFT")
    fp.set_pair_ts_diff_max(15,1969,"LEFT")
    fp.set_pair_filtering_en(14,1,"LEFT")
    fp.set_pair_ts_diff_min(14,99,"LEFT")
    fp.set_pair_ts_diff_max(14,1998,"LEFT")
    fp.set_pair_filtering_en(13,1,"LEFT")
    fp.set_pair_ts_diff_min(13,108,"LEFT")
    fp.set_pair_ts_diff_max(13,2027,"LEFT")
    fp.set_pair_filtering_en(12,1,"LEFT")
    fp.set_pair_ts_diff_min(12,112,"LEFT")
    fp.set_pair_ts_diff_max(12,2055,"LEFT")
    fp.set_pair_filtering_en(11,1,"LEFT")
    fp.set_pair_ts_diff_min(11,131,"LEFT")
    fp.set_pair_ts_diff_max(11,2080,"LEFT")
    fp.set_pair_filtering_en(10,1,"LEFT")
    fp.set_pair_ts_diff_min(10,143,"LEFT")
    fp.set_pair_ts_diff_max(10,2099,"LEFT")
    fp.set_pair_filtering_en(9,1,"LEFT")
    fp.set_pair_ts_diff_min(9,157,"LEFT")
    fp.set_pair_ts_diff_max(9,2119,"LEFT")
    fp.set_pair_filtering_en(8,1,"LEFT")
    fp.set_pair_ts_diff_min(8,171,"LEFT")
    fp.set_pair_ts_diff_max(8,2139,"LEFT")
    fp.set_pair_filtering_en(7,1,"LEFT")
    fp.set_pair_ts_diff_min(7,181,"LEFT")
    fp.set_pair_ts_diff_max(7,2156,"LEFT")
    fp.set_pair_filtering_en(6,1,"LEFT")
    fp.set_pair_ts_diff_min(6,200,"LEFT")
    fp.set_pair_ts_diff_max(6,2174,"LEFT")
    fp.set_pair_filtering_en(5,1,"LEFT")
    fp.set_pair_ts_diff_min(5,215,"LEFT")
    fp.set_pair_ts_diff_max(5,2188,"LEFT")
    fp.set_pair_filtering_en(4,1,"LEFT")
    fp.set_pair_ts_diff_min(4,229,"LEFT")
    fp.set_pair_ts_diff_max(4,2201,"LEFT")
    fp.set_pair_filtering_en(3,1,"LEFT")
    fp.set_pair_ts_diff_min(3,244,"LEFT")
    fp.set_pair_ts_diff_max(3,2215,"LEFT")
    fp.set_pair_filtering_en(2,1,"LEFT")
    fp.set_pair_ts_diff_min(2,259,"LEFT")
    fp.set_pair_ts_diff_max(2,2230,"LEFT")
    fp.set_pair_filtering_en(1,1,"LEFT")
    fp.set_pair_ts_diff_min(1,273,"LEFT")
    fp.set_pair_ts_diff_max(1,2243,"LEFT")
    fp.set_pair_filtering_en(0,1,"LEFT")
    fp.set_pair_ts_diff_min(0,287,"LEFT")
    fp.set_pair_ts_diff_max(0,2256,"LEFT")
    fp.set_pair_filtering_en(15,1,"MIDDLE")
    fp.set_pair_ts_diff_min(15,303,"MIDDLE")
    fp.set_pair_ts_diff_max(15,2271,"MIDDLE")
    fp.set_pair_filtering_en(14,1,"MIDDLE")
    fp.set_pair_ts_diff_min(14,317,"MIDDLE")
    fp.set_pair_ts_diff_max(14,2285,"MIDDLE")
    fp.set_pair_filtering_en(13,1,"MIDDLE")
    fp.set_pair_ts_diff_min(13,331,"MIDDLE")
    fp.set_pair_ts_diff_max(13,2298,"MIDDLE")
    fp.set_pair_filtering_en(12,1,"MIDDLE")
    fp.set_pair_ts_diff_min(12,347,"MIDDLE")
    fp.set_pair_ts_diff_max(12,2313,"MIDDLE")
    fp.set_pair_filtering_en(11,1,"MIDDLE")
    fp.set_pair_ts_diff_min(11,361,"MIDDLE")
    fp.set_pair_ts_diff_max(11,2327,"MIDDLE")
    fp.set_pair_filtering_en(10,1,"MIDDLE")
    fp.set_pair_ts_diff_min(10,375,"MIDDLE")
    fp.set_pair_ts_diff_max(10,2340,"MIDDLE")
    fp.set_pair_filtering_en(9,1,"MIDDLE")
    fp.set_pair_ts_diff_min(9,390,"MIDDLE")
    fp.set_pair_ts_diff_max(9,2355,"MIDDLE")
    fp.set_pair_filtering_en(8,1,"MIDDLE")
    fp.set_pair_ts_diff_min(8,404,"MIDDLE")
    fp.set_pair_ts_diff_max(8,2368,"MIDDLE")
    fp.set_pair_filtering_en(7,1,"MIDDLE")
    fp.set_pair_ts_diff_min(7,419,"MIDDLE")
    fp.set_pair_ts_diff_max(7,2382,"MIDDLE")
    fp.set_pair_filtering_en(6,1,"MIDDLE")
    fp.set_pair_ts_diff_min(6,434,"MIDDLE")
    fp.set_pair_ts_diff_max(6,2397,"MIDDLE")
    fp.set_pair_filtering_en(5,1,"MIDDLE")
    fp.set_pair_ts_diff_min(5,448,"MIDDLE")
    fp.set_pair_ts_diff_max(5,2410,"MIDDLE")
    fp.set_pair_filtering_en(4,1,"MIDDLE")
    fp.set_pair_ts_diff_min(4,462,"MIDDLE")
    fp.set_pair_ts_diff_max(4,2424,"MIDDLE")
    fp.set_pair_filtering_en(3,1,"MIDDLE")
    fp.set_pair_ts_diff_min(3,477,"MIDDLE")
    fp.set_pair_ts_diff_max(3,2439,"MIDDLE")
    fp.set_pair_filtering_en(2,1,"MIDDLE")
    fp.set_pair_ts_diff_min(2,491,"MIDDLE")
    fp.set_pair_ts_diff_max(2,2452,"MIDDLE")
    fp.set_pair_filtering_en(1,1,"MIDDLE")
    fp.set_pair_ts_diff_min(1,505,"MIDDLE")
    fp.set_pair_ts_diff_max(1,2466,"MIDDLE")
    fp.set_pair_filtering_en(0,1,"MIDDLE")
    fp.set_pair_ts_diff_min(0,520,"MIDDLE")
    fp.set_pair_ts_diff_max(0,2481,"MIDDLE")
    fp.set_pair_filtering_en(15,1,"RIGHT")
    fp.set_pair_ts_diff_min(15,535,"RIGHT")
    fp.set_pair_ts_diff_max(15,2494,"RIGHT")
    fp.set_pair_filtering_en(14,1,"RIGHT")
    fp.set_pair_ts_diff_min(14,549,"RIGHT")
    fp.set_pair_ts_diff_max(14,2508,"RIGHT")
    fp.set_pair_filtering_en(13,1,"RIGHT")
    fp.set_pair_ts_diff_min(13,564,"RIGHT")
    fp.set_pair_ts_diff_max(13,2523,"RIGHT")
    fp.set_pair_filtering_en(12,1,"RIGHT")
    fp.set_pair_ts_diff_min(12,578,"RIGHT")
    fp.set_pair_ts_diff_max(12,2537,"RIGHT")
    fp.set_pair_filtering_en(11,1,"RIGHT")
    fp.set_pair_ts_diff_min(11,592,"RIGHT")
    fp.set_pair_ts_diff_max(11,2550,"RIGHT")
    fp.set_pair_filtering_en(10,1,"RIGHT")
    fp.set_pair_ts_diff_min(10,606,"RIGHT")
    fp.set_pair_ts_diff_max(10,2564,"RIGHT")
    fp.set_pair_filtering_en(9,1,"RIGHT")
    fp.set_pair_ts_diff_min(9,616,"RIGHT")
    fp.set_pair_ts_diff_max(9,2574,"RIGHT")
    fp.set_pair_filtering_en(8,1,"RIGHT")
    fp.set_pair_ts_diff_min(8,625,"RIGHT")
    fp.set_pair_ts_diff_max(8,2583,"RIGHT")
    fp.set_pair_filtering_en(7,1,"RIGHT")
    fp.set_pair_ts_diff_min(7,632,"RIGHT")
    fp.set_pair_ts_diff_max(7,2590,"RIGHT")
    fp.set_pair_filtering_en(6,1,"RIGHT")
    fp.set_pair_ts_diff_min(6,636,"RIGHT")
    fp.set_pair_ts_diff_max(6,2593,"RIGHT")
    fp.set_pair_filtering_en(5,1,"RIGHT")
    fp.set_pair_ts_diff_min(5,641,"RIGHT")
    fp.set_pair_ts_diff_max(5,2597,"RIGHT")
    fp.set_pair_filtering_en(4,1,"RIGHT")
    fp.set_pair_ts_diff_min(4,641,"RIGHT")
    fp.set_pair_ts_diff_max(4,2597,"RIGHT")
    fp.set_pair_filtering_en(3,1,"RIGHT")
    fp.set_pair_ts_diff_min(3,640,"RIGHT")
    fp.set_pair_ts_diff_max(3,2597,"RIGHT")
    fp.set_pair_filtering_en(2,1,"RIGHT")
    fp.set_pair_ts_diff_min(2,640,"RIGHT")
    fp.set_pair_ts_diff_max(2,2596,"RIGHT")
    fp.set_pair_filtering_en(1,1,"RIGHT")
    fp.set_pair_ts_diff_min(1,640,"RIGHT")
    fp.set_pair_ts_diff_max(1,2596,"RIGHT")
    fp.set_pair_filtering_en(0,1,"RIGHT")
    fp.set_pair_ts_diff_min(0,638,"RIGHT")
    fp.set_pair_ts_diff_max(0,2595,"RIGHT")

def prepare_re31_state(state,version,firmware="4.8",ccomp=15,delay_reset=4):
    """!
    Prepare a state from a newly created one before the pedestal procedure

    @param state Name of the state
    @param version version number
    @param firmware FEB firmware version ("4.8")
    @param ccomp PA_CCOMP value for all ASICS
    @param delay_reset delay_reset_trigge value for all ASICs

    @remark
    The pair filtering is initialised for RE3.1 but disable
    The MUTE Finite State Machine is disabled
    A new state is uploaded
    """
    s=cra.instance()
    # Download
    s.download_setup(state,version)
    # Set firmware version
    f1=s.setup.febs[0]
    f1.set_fpga_version(firmware)
    #Change PA_CCOMP and auto reset
    p=s.setup.febs[0].petiroc
    p.set_parameter("pa_ccomp",ccomp,asic=None)
    p.set_parameter("delay_reset_trigger",delay_reset,asic=None)
    # Load pair filtering values
    fp=s.setup.febs[0].fpga
    set_pair_filtering_re31(fp)
    # Disable Pair filtering  and MUTE FSM for pedestals
    fp.set_parameter("DATA_PATH_CTRL.RETRIG_MITIG_MUTEROC_TIME.STEP_120MHz",0,fpga=None)
    for i in range(16):
        fp.set_pair_filtering_en(i,0,"LEFT")
        fp.set_pair_filtering_en(i,0,"MIDDLE")
        fp.set_pair_filtering_en(i,0,"RIGHT")
    #Upload the state
    s.upload_changes(f"FPGA version {firmware}/CCOMP {ccomp}/pair range set/filtering Off")

def enable_mute_fsm(fp,threshold=3,mute=4,decrement=11):
    """!
    Enable or change  MUTE FSM for one feb_fpga_+registers object

    @param fp the fpga register object
    @param threshold the DATA_PATH_CTRL.RETRIG_MITIG_THRESHOLD.COUNT
    @param mute The DATA_PATH_CTRL.RETRIG_MITIG_MUTEROC_TIME.STEP_120MHz
    @param decrement The DATA_PATH_CTRL.RETRIG_MITIG_DECREMENT_TIME.STEP_120MHz
    """
    fp.set_parameter("DATA_PATH_CTRL.RETRIG_MITIG_THRESHOLD.COUNT",threshold,fpga=None)
    fp.set_parameter("DATA_PATH_CTRL.RETRIG_MITIG_MUTEROC_TIME.STEP_120MHz",mute,fpga=None)
    fp.set_parameter("DATA_PATH_CTRL.RETRIG_MITIG_DECREMENT_TIME.STEP_120MHz",decrement,fpga=None)

def enable_pair_filtering(fp):
    """!
    Enable pair filtering  for one feb_fpga_+registers object

    @param fp the fpga register object

    """
    for i in range(16):
        fp.set_pair_filtering_en(i,1,"LEFT")
        fp.set_pair_filtering_en(i,1,"MIDDLE")
        fp.set_pair_filtering_en(i,1,"RIGHT")


def shift_re31_strips_0_1(p,dac_shift=-2):
    """!
    Shift DAC6B for strips 0 and 1 of RE31 (Noisy edges)

    @param p feb_petiroc_registers object
    @param dac_shift The additive shift to 6bDAC
    """
    p.correct_6b_dac(5,dac_shift,"LEFT_BOT")
    p.correct_6b_dac(6,dac_shift,"LEFT_BOT")
    p.correct_6b_dac(25,dac_shift,"LEFT_TOP")
    p.correct_6b_dac(26,dac_shift,"LEFT_TOP")

def set_asymetric_threshold(p,d_shift=3):
    """!
    Shift DAC10B for High Radius side (TOP Asics) 

    @param p feb_petiroc_registers object
    @param d_shift The additive shift to High Radius
    """
    t_lt=p.get_parameter("10b_dac_vth_discri_time","LEFT_TOP")
    p.set_parameter("10b_dac_vth_discri_time",int(t_lt+d_shift),"LEFT_TOP")
    t_mt=p.get_parameter("10b_dac_vth_discri_time","MIDDLE_TOP")
    p.set_parameter("10b_dac_vth_discri_time",int(t_mt+d_shift),"MIDDLE_TOP")
    t_rt=p.get_parameter("10b_dac_vth_discri_time","RIGHT_TOP")
    p.set_parameter("10b_dac_vth_discri_time",int(t_rt+d_shift),"RIGHT_TOP")
    


def modify_petiroc(name,version):
    s=cra.instance()
    s.download_setup(name,version)
    p=s.setup.febs[0].petiroc
    p.set_mask_discri_time(2,1,"MIDDLE_TOP")
    p.set_input_dac(2,0x4)
    p.set_6b_dac(3,23)
    p.set_parameter("10b_dac_vth_discri_time",320,"LEFT_BOT")
    p.set_parameter("pa_ccomp",0b0111,"LEFT_TOP")
    s.upload_changes("PETIROC params modified")
    return s.setup
def modify_petiroc_from_file(name,version,csvfile):
    s=cra.instance()
    s.download_setup(name,version)
    p=s.setup.febs[0].petiroc
    p.load_from_csv_file(csvfile)
    s.upload_changes("PETIROC params from %s" % csvfile)
    return s.setup

def modify_fpga(name,version):
    s=cra.instance()
    s.download_setup(name,version)
    pf=s.setup.febs[0].fpga
    pf.set_pair_ts_diff_max(3,12000,"RIGHT")
    pf.set_pair_ts_diff_max(4,12000,"MIDDLE")
    pf.set_pair_ts_diff_max(5,12000,"LEFT")
    s.upload_changes("FPGA params modified")
    return s.setup

def ls_setups():
    s=cra.instance()
    s.list_setups()
    s.download_setup("TEST_SOFT",6)
    print(s.setup.febs[0].fpga_version)
    print(s.setup.febs[0].petiroc_version)
if __name__ == "__main__":



    ls_setups()
    """
    # Create a new state TEST_SOFT with one feb #5
    create_a_state("TEST_SOFT",5)
    # download it and write csv files TEST_SOFT_1_f_5...csv on /dev/shm/feb_csv/
    download_a_state("TEST_SOFT",1)
    # Change some petiroc values from TEST_SOFT:1
    modify_petiroc("TEST_SOFT",1)
    # download new state (version 2)
    download_a_state("TEST_SOFT",2)
    # modify FPGA register from state 2
    modify_fpga("TEST_SOFT",2)
    # download state 3
    download_a_state("TEST_SOFT",3)
    # modify FPGA registers from state 1
    modify_fpga("TEST_SOFT",1)
    #download state 4
    download_a_state("TEST_SOFT",4)
    #Modify a state with a csv file
    modify_petiroc_from_file("TEST_SOFT",4,"./modified_petiroc.csv")
    # download state 5
    download_a_state("TEST_SOFT",5)

    """
