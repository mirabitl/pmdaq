export LD_LIBRARY_PATH=/usr/local/pmdaq/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/pmdaq/bin:$PATH
export PYTHONPATH=/usr/local/pmdaq/share:$PYTHONPATH

alias restart_pmdaq='sudo /usr/local/pmdaq/bin/pmdaemon stop;sudo /usr/local/pmdaq/bin/pmdaemon start'
alias restart_pns='sudo /usr/local/pmdaq/bin/pnsdaemon stop;sudo /usr/local/pmdaq/bin/pnsdaemon start'

alias CONFIGURE='combdaq --daq-configure'
alias INIT='combdaq --daq-initialise'
alias SPY='combdaq --daq-status'
alias REMOVE='combdaq --jc-destroy'
alias VIEW='combdaq --jc-info'
alias REGISTER='combdaq --daq-register'
alias RUN='combdaq --daq-start'
alias STOP='combdaq --daq-stop'
alias RESUME='combdaq --trig-resume'
alias PAUSE='combdaq --trig-pause'
