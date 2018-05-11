


# ESPWifiManager
WifiManager extended.....

based on:
#WifiManager by tzapu (https://github.com/tzapu/WiFiManager)

inspired by :
- the config and server handling from tasmota (sonoff)
- marquee-scroller

#############################################################################

This works with the ESP8266 Arduino platform with a recent stable release(2.0.0 or newer) 
https://github.com/esp8266/Arduino


- suitable for all 4MB ESP8266 devices, like the NodeMCU, ESP8266xx ....
- WifiManager functionality for first use of ESP device
  - AP creation
  - Wifi scan
  - Wifi connect
  - mqtt connect
  - AP shutdown
  
#############################################################################

EXTENDED:
 after successfull connect:
- start Webserver on Port 80
- show Basic information
- restart/reconnect/reset ESP

#############################################################################

ToDo/Wishlist:
- mqtt handling: edit config, 
- integrate modules:
 - gpio on/off
 - MX72xx integration
 - OLED integration

 #############################################################################
 
 (c) 2018 by highfishGit
 
