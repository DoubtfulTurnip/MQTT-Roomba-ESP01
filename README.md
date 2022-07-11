# Roomba 600 series ESP01 modification

Add an ESP01 to a Roomba 600 series vacuum to make it wireless capable. 

Using MQTT it is possible to then integrate with Home Assistant

## Changes made from the original repo

* Added an OTA capability using AsyncElegantOTA
  - Included a username and password to provide basic protection for the web interface (this needs to be specified before installation)
  - Just browse to http://{ipaddress}/update to access

* Added the stop function for the Roomba
* Changed MDI icon to updated naming within the customize yaml
* Added my own configuration yaml for examples of buttons instead of switches


## Forked from "The Hook Up" git repo 
An arduino program to add MQTT commands to your roomba 500 or 600 series.

https://www.youtube.com/watch?v=t2NgA8qYcFI

https://github.com/thehookup/MQTT-Roomba-ESP01
