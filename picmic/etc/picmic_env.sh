# ROOT PATH
export ROOTSYS=/opt/root
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
export PATH=$ROOTSYS/bin:$PATH
export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH
# PMDAQ PATH
source /usr/local/pmdaq/etc/pmdaq.bashrc

#Drivers
export LIROCDRV=${HOME}/liroc_picotdc_daq/software/
export PICMICSW=/opt/pmdaq/picmic/

#PYTHONPATH
export PYTHONPATH=${LIROCDRV}:${PICMICSW}/bin:${PICMICSW}/db:${PICMICSW}/share:$PYTHONPATH

#PATH
export PATH=${PICMICSW}/bin:${PICMICSW}/db:${PICMICSW}/share:${PICMICSW}/tk:$PATH


# DB
export MGDBLOGIN=axxxxc/Rxxxxxx8@lyoxxxxxx01:27017@PICMIC

export PYENV=${HOME}/testtkenv

. ${PYENV}/bin/activate
