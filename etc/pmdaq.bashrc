export LD_LIBRARY_PATH=/usr/local/pmdaq/lib:$LD_LIBRARY_PATH
export PATH=/usr/local/pmdaq/bin:$PATH
export PYTHONPATH=/usr/local/pmdaq/share:$PYTHONPATH

alias restart_pmdaq='sudo service pmdaq restart'
alias restart_pns='sudo service pns restart'

alias CONFIGURE='combdaq --daq-configure'
alias INIT='combdaq --daq-initialise'
alias SPY='combdaq --daq-status'
alias REMOVE='combdaq --jc-destroy'
alias RESTART='combdaq --daq-restart'
alias VIEW='combdaq --jc-info'
alias REGISTER='combdaq --daq-register'
alias RUN='combdaq --daq-start'
alias STOP='combdaq --daq-stop'
alias RESUME='combdaq --trig-resume'
alias PAUSE='combdaq --trig-pause'
alias spylog='ps -laef | grep pmd | awk '\''{print "tail -f /tmp/pmd"$4".log"}'\'' | /bin/bash - '
