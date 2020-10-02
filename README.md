# RCO_View

RCO View is a program that can connect to multiple RCOs over the RCE serial data connection and query the radios, as well as power status for sites that are on DC.

# Instructions
* Flash ubuntu server on to sd card
* Set ubuntu password
  * Default username is ubuntu default password is ubuntu. It will say incorrect password until the cloud service thing initializes. Set password to "rcomonitor".
* Update netplan using file stored with flash software
* Generate ssh key with "sudo ssh-keygen -A" and reboot
* Open rasp pi in vscode
* Create bin folder in home directory
* Copy everything from firmware folder to the home folder (/home/ubuntu)
* chmod +x to the firmware file located in bin directory
* Create shortcut to firmware called RCO_Monitor
  * "cd /home/ubuntu/bin && ln -s ./RCO_Monitor-vX.X.X.rpi ./RCO_Monitor"
* Create the system service
  * "sudo systemctl link /home/ubuntu/RCO_Monitor.service"
  * "sudo systemctl enable RCO_Monitor"
* reboot - verify program is running:
  * "sudo systemctl status RCO_Monitor"
* Connect to rce and verify can get firmware version
