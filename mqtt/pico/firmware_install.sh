#!/bin/bash
cd /opt/pmdaq/mqtt/pico

ampy --port /dev/ttyACM0 put bme280.py
ampy --port /dev/ttyACM0 put cpwplus.py
ampy --port /dev/ttyACM0 put freesans12.py
ampy --port /dev/ttyACM0 put genesysPico.py
ampy --port /dev/ttyACM0 put hih81310.py
ampy --port /dev/ttyACM0 put oled.py
ampy --port /dev/ttyACM0 put writer.py
ampy --port /dev/ttyACM0 put zupPico.py
ampy --port /dev/ttyACM0 put brooksPico.py

cd /opt/pmdaq/scripts/utils
ampy --port /dev/ttyACM0 put zupInterface.py
ampy --port /dev/ttyACM0 put genesysInterface.py
ampy --port /dev/ttyACM0 put hpuni.py
ampy --port /dev/ttyACM0 put brooksInterface2.py
