#!/bin/bash
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:/opt/pmdaq/lib:$LD_LIBRARY_PATH
/opt/pmdaq/bin/pmd -H `hostname` -P 7777
