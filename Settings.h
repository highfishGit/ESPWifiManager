///////////////////////////////////////////////////////////////////////////////
//MIT License
//
//Copyright (c) 2018 Daniel Lienhard
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
///////////////////////////////////////////////////////////////////////////////

//load all libs we need
#include <FS.h>  

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>  
#include <PubSubClient.h>
#include "TimeClient.h"

//******************************
// Start Settings
//******************************
const int WEBSERVER_PORT = 80; // The port you can access this device on over HTTP
const boolean WEBSERVER_ENABLED = true;  // Device will provide a web interface via http://[ip]:[port]/
char* www_username = "admin";  // User account for the Web Interface
char* www_password = "password";  // Password for the Web Interface
float UtcOffset = +2; // Hour offset from GMT for your timezone
int minutesBetweenDataRefresh = 60;


//default custom static IP for desired network
char static_ip[16] = "192.168.xxx.xxx";
char static_gw[16] = "192.168.xxx.xxx";
char static_sn[16] = "255.255.255.0";

 


//ota
boolean ENABLE_OTA = true;  // this will allow you to load firmware to the device over WiFi (see OTA for ESP8266)

//******************************
// End Settings
//******************************

String themeColor = "deep-orange"; // this can be changed later in the web interface.
