export LD_LIBRARY_PATH=/opt/cactus/lib/:/usr/local/pmdaq/lib/:/usr/local/cms_febv2/lib:$LD_LIBRARY_PATH
export DRVFEBV2=/home/acqilc/feb-backend-emulator/Python_project/
export LIGHTFEBV2=/home/acqilc/alinx/cms-irpc-feb-lightdaq-dev/software/cms_irpc_feb_lightdaq/

export DAQFEBV2=/usr/local/cms_febv2
export MGDBLOGIN=acqcmsmu/OpacIP2I@lyoilcdaq01:27017@FEBROC
export PYTHONPATH=${DAQFEBV2}/bin:${DAQFEBV2}/db:${DAQFEBV2}/daq:${DAQFEBV2}/light_daq:${DRVFEBV2}:${LIGHTFEBV2}:$PYTHONPATH
#${HOME}/febv2_access/:../:$PYTHONPATH 
export PATH=.:${DAQFEBV2}/bin:${DAQFEBV2}/db:${DAQFEBV2}/daq:${DAQFEBV2}/light_daq:$PATH 
