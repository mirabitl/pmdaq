export LD_LIBRARY_PATH=/usr/local/pmdaq/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/pmdaq/bin:$PATH
export PYTHONPATH=/usr/local/pmdaq/share:$PYTHONPATH

alias CONFIGURE='combdaq --daq-configure'
alias INIT='combdaq --daq-initialise'
alias SPY='combdaq --daq-status'
alias REMOVE='combdaq --jc-destroy'
alias VIEW='combdaq --jc-info'
alias REGISTER='combdaq --jc-info'
alias RUN='combdaq --daq-start'
alias STOP='combdaq --daq-stop'
alias RESUME='combdaq --trig-resume'
alias PAUSE='combdaq --trig-pause'
