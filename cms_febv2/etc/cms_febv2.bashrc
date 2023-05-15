export LD_LIBRARY_PATH=/opt/cactus/lib/:/usr/local/cms_febv2/lib:$LD_LIBRARY_PATH
export DRVFEBV2=/home/acqilc/feb-backend-emulator/Python_project/
export DAQFEBV2=/usr/local/cms_febv2
export MGDBLOGIN=acxxxxmu/OxxxxxI@lyocmsmu06:27017@FEBROC
export PYTHONPATH=${DAQFEBV2}/bin:${DAQFEBV2}/db:${DAQFEBV2}/daq:${DRVFEBV2}:$PYTHONPATH
#${HOME}/febv2_access/:../:$PYTHONPATH 
export PATH=.:${DAQFEBV2}/bin:${DAQFEBV2}/db:${DAQFEBV2}/daq:$PATH 
