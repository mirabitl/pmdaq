#!/bin/bash
sudo systemctl disable socket_receiver 
sudo service socket_receiver stop
make
sudo cp socket_receiver.json /etc
sudo cp pfring_rcv /usr/local/bin/
sudo cp socket_receiver_service_daemon /usr/local/bin/
sudo cp socket_receiver.service /lib/systemd/system/
sudo systemctl enable socket_receiver 
sudo service socket_receiver stop
