export LD_LIBRARY_PATH=/opt/cactus/lib/:/opt/pmdaq/cms_febv2/daq:$LD_LIBRARY_PATH
export DRVFEBV2=${HOME}/feb-backend-emulator/Python_project/
export DAQFEBV2=/opt/pmdaq/cms_febv2
export MGDBLOGIN=acqcmsmu/OpacIP2I@lyocmsmu06:27017@FEBROC
export PYTHONPATH=${DAQFEBV2}:${DAQFEBV2}/db:${DAQFEBV2}/daq:${DRVFEBV2}:$PYTHONPATH
#${HOME}/febv2_access/:../:$PYTHONPATH 
export PATH=.:${DAQFEBV2}:${DAQFEBV2}/db:${DAQFEBV2}/daq:$PATH 
