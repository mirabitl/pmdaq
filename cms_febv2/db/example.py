import csv_register_access as cra

def create_a_state(name,febid):
    s=cra.instance()
    s.create_setup(name)
    
    f1=cra.febv2_registers(febid)
    f1.load_defaults()
    s.setup.add_febv2(f1)
    s.upload_changes("Un state avec les valuers par defaut")
    return s.setup
def download_a_state(name,version):
    s=cra.instance()
    s.download_setup(name,version)
    s.to_csv_files()
    return s.setup

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
