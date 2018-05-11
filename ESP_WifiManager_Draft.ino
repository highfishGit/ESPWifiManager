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


//Edit the settings file to your needs
#include "Settings.h"

#define HOSTNAME "ESP8266-" 
#define CONFIG "/conf.txt"
#define VERSION "0.9"

// Time 
TimeClient timeClient(UtcOffset);
long lastEpoch = 0;
long firstEpoch = 0;
long displayOffEpoch = 0;
String lastMinute = "xx";
String lastSecond = "xx";
String lastReportStatus = "";
boolean displayOn = true;


//mqtt
const char* mqtt_server_own = "192.168.178.xxx";
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_user[40];
char mqtt_pass[20];
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


// id/name, placeholder/prompt, default, length
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);

//flag for saving data
bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//get time
void getUpdateTime() {

  Serial.println();
  Serial.println("Updating Time...");
  //Update the Time
  timeClient.updateTime();
  lastEpoch = timeClient.getCurrentEpoch();
  Serial.println("Local time: " + timeClient.getFormattedTime());
}


void readSettings() {
  if (SPIFFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeSettings();
    return;
  }
  File fr = SPIFFS.open(CONFIG, "r");
  String line;
  while(fr.available()) {
    line = fr.readStringUntil('\n');

    if (line.indexOf("UtcOffset=") >= 0) {
      UtcOffset = line.substring(line.lastIndexOf("UtcOffset=") + 10).toFloat();
      Serial.println("UtcOffset=" + String(UtcOffset));
    }
 //   if (line.indexOf("octoKey=") >= 0) {
 //     OctoPrintApiKey = line.substring(line.lastIndexOf("octoKey=") + 8);
 //     OctoPrintApiKey.trim();
 //     Serial.println("OctoPrintApiKey=" + OctoPrintApiKey);
//    }
//    if (line.indexOf("octoServer=") >= 0) {
//      OctoPrintServer = line.substring(line.lastIndexOf("octoServer=") + 11);
 //     OctoPrintServer.trim();
//      Serial.println("OctoPrintServer=" + OctoPrintServer);
//    }
//    if (line.indexOf("octoPort=") >= 0) {
//      OctoPrintPort = line.substring(line.lastIndexOf("octoPort=") + 9).toInt();
 //     Serial.println("OctoPrintPort=" + String(OctoPrintPort));
//    }
    if (line.indexOf("refreshRate=") >= 0) {
      minutesBetweenDataRefresh = line.substring(line.lastIndexOf("refreshRate=") + 12).toInt();
      Serial.println("minutesBetweenDataRefresh=" + String(minutesBetweenDataRefresh));
    }
    if (line.indexOf("themeColor=") >= 0) {
      themeColor = line.substring(line.lastIndexOf("themeColor=") + 11);
      themeColor.trim();
      Serial.println("themeColor=" + themeColor);
    }
    if (line.indexOf("www_username=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_username=") + 13);
      temp.trim();
      temp.toCharArray(www_username, sizeof(temp));
      Serial.println("www_username=" + String(www_username));
    }
    if (line.indexOf("www_password=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_password=") + 13);
      temp.trim();
      temp.toCharArray(www_password, sizeof(temp));
      Serial.println("www_password=" + String(www_password));
    }
  }
  fr.close();
  timeClient.setUtcOffset(UtcOffset);
 // printerClient.updateOctoPrintClient(OctoPrintApiKey, OctoPrintServer, OctoPrintPort);
 // printerClient.getPrinterJobResults();
}

void writeSettings() {
  // Save decoded message to SPIFFS file for playback on power up.
  File f = SPIFFS.open(CONFIG, "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("UtcOffset=" + String(UtcOffset));
  //  f.println("octoKey=" + OctoPrintApiKey);
  //  f.println("octoServer=" + OctoPrintServer);
 //   f.println("octoPort=" + String(OctoPrintPort));
    f.println("refreshRate=" + String(minutesBetweenDataRefresh));
    f.println("themeColor=" + themeColor);
    f.println("www_username=" + String(www_username));
    f.println("www_password=" + String(www_password));
  }
  f.close();
  readSettings();
  timeClient.setUtcOffset(UtcOffset);
}


//declairing prototypes
int8_t getWifiQuality();

//webserver
ESP8266WebServer server(WEBSERVER_PORT);
//webserver file 
const String WEB_ACTIONS =  "<a class='w3-bar-item w3-button' href='/'><i class='fa fa-home'></i> Home</a>"
                            "<a class='w3-bar-item w3-button' href='/configure'><i class='fa fa-cog'></i> Configure</a>"
                            "<a class='w3-bar-item w3-button' href='/systemreset' onclick='return confirm(\"Do you want to reset to default settings?\")'><i class='fa fa-undo'></i> Reset Settings</a>"
                            "<a class='w3-bar-item w3-button' href='/forgetwifi' onclick='return confirm(\"Do you want to forget to WiFi connection?\")'><i class='fa fa-wifi'></i> Forget WiFi</a>"
                            "<a class='w3-bar-item w3-button' href='https://github.com/Qrome' target='_blank'><i class='fa fa-question-circle'></i> About</a>";
                            
const String CHANGE_FORM =  "<form class='w3-container' action='/updateconfig' method='get'><h2>Station Config:</h2>"
                            "Time Refresh (minutes) <select class='w3-option w3-padding' name='refresh'>%OPTIONS%</select></p>"
                            "Theme Color <select class='w3-option w3-padding' name='theme'>%THEME_OPTIONS%</select></p>"
                            "<label>UTC Time Offset</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='utcoffset' value='%UTCOFFSET%' maxlength='12'>"
                            "<label>User ID (for this interface)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='userid' value='%USERID%' maxlength='20'>"
                            "<label>Password </label><input class='w3-input w3-border w3-margin-bottom' type='password' name='stationpassword' value='%STATIONPASSWORD%'>"
                            "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='submit'>Save</button></form>";

const String COLOR_THEMES = "<option>red</option>"
                            "<option>pink</option>"
                            "<option>purple</option>"
                            "<option>deep-purple</option>"
                            "<option>indigo</option>"
                            "<option>blue</option>"
                            "<option>light-blue</option>"
                            "<option>cyan</option>"
                            "<option>teal</option>"
                            "<option>green</option>"
                            "<option>light-green</option>"
                            "<option>lime</option>"
                            "<option>khaki</option>"
                            "<option>yellow</option>"
                            "<option>amber</option>"
                            "<option>orange</option>"
                            "<option>deep-orange</option>"
                            "<option>blue-grey</option>"
                            "<option>brown</option>"
                            "<option>grey</option>"
                            "<option>dark-grey</option>"
                            "<option>black</option>"
                            "<option>w3schools</option>";
String getHeader() {
  String menu = String(WEB_ACTIONS);

  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>ESPWifiManager</title><link rel='icon' href='data:;base64,='>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/lib/w3-theme-" + themeColor + ".css'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>";
  html += "</head><body>";
  html += "<nav class='w3-sidebar w3-bar-block w3-card' style='margin-top:88px' id='mySidebar'>";
  html += "<div class='w3-container w3-theme-d2'>";
  html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright w3-large'><i class='fa fa-times'></i></span>";
  html += "<div class='w3-cell w3-left w3-xxxlarge' style='width:60px'><i class='fa fa-cube'></i></div>";
  html += "<div class='w3-padding'>Menu</div></div>";
  html += menu;
  html += "</nav>";
  html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item w3-button w3-xxxlarge w3-hover-theme' onclick='openSidebar()'><i class='fa fa-bars'></i></button><h2 class='w3-bar-item'>ESPWifiManagerExtended</h2></header>";
  html += "<script>";
  html += "function openSidebar(){document.getElementById('mySidebar').style.display='block'}function closeSidebar(){document.getElementById('mySidebar').style.display='none'}closeSidebar();";
  html += "</script>";
  html += "<br><div class='w3-container w3-large' style='margin-top:88px'>";
  return html;
}

String getFooter() {
//  int8_t rssi = getWifiQuality();
  
  String html = "<br><br><br>";
  html += "</div>";
  html += "<footer class='w3-container w3-bottom w3-theme w3-margin-top'>";
  if (lastReportStatus != "") {
    html += "<i class='fa fa-external-link'></i> Report Status: " + lastReportStatus + "<br>";
  }
  html += "<i class='fa fa-paper-plane-o'></i> Version: " + String(VERSION) + "<br>";
  html += "<i class='fa fa-rss'></i> Signal Strength: ";
//  html += String(rssi) + "%";
  html += "</footer>";
  html += "</body></html>";
  return html;
}

void displayHome() {

  String html = "";
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(String(getHeader()));
  html += "<div class='w3-cell-row' style='width:100%'><h2>Time: " + timeClient.getHours() + ":" + timeClient.getMinutes() + ":" + timeClient.getSeconds() + " " + timeClient.getAmPm() + "</h2></div><div class='w3-cell-row'>";
  html += "<div class='w3-cell w3-container' style='width:100%'><p>";
  html += "--here is the place where you could put your usecase...like gpio on/off,led,matrix,oled,servo <br>";
  server.sendContent(String(html)); // spit out what we got
  server.sendContent(String(getFooter()));
  server.sendContent("");
  server.client().stop();
}

void redirectHome() {
  // Send them back to the Root Directory
  server.sendHeader("Location", String("/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
}

void handleSystemReset() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  Serial.println("Reset System Configuration");
  if (SPIFFS.remove(CONFIG)) {
    redirectHome();
    ESP.restart();
  }
}



void handleWifiReset() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  redirectHome();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void handleUpdateConfig() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  //OctoPrintApiKey = server.arg("octoPrintApiKey");
  //OctoPrintServer = server.arg("octoPrintAddress");
  //OctoPrintPort = server.arg("octoPrintPort").toInt();
  minutesBetweenDataRefresh = server.arg("refresh").toInt();
  themeColor = server.arg("theme");
  UtcOffset = server.arg("utcoffset").toFloat();
  String temp = server.arg("userid");
  temp.toCharArray(www_username, sizeof(temp));
  temp = server.arg("stationpassword");
  temp.toCharArray(www_password, sizeof(temp));
  writeSettings();
  lastEpoch = 0;
  redirectHome();
}

void handleConfigure() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  } 
  String html = "";
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  html = getHeader();
  server.sendContent(html); 
  String form = String(CHANGE_FORM);
  String options = "<option>10</option><option>15</option><option>20</option><option>30</option><option>60</option>";
  options.replace(">"+String(minutesBetweenDataRefresh)+"<", " selected>"+String(minutesBetweenDataRefresh)+"<");
  form.replace("%OPTIONS%", options);
  String themeOptions = String(COLOR_THEMES);
  themeOptions.replace(">"+String(themeColor)+"<", " selected>"+String(themeColor)+"<");
  form.replace("%THEME_OPTIONS%", themeOptions);
  form.replace("%UTCOFFSET%", String(UtcOffset));
  form.replace("%USERID%", String(www_username));
  form.replace("%STATIONPASSWORD%", String(www_password));

  server.sendContent(String(form));
  
  html = getFooter();
  server.sendContent(html);
  server.sendContent("");
  server.client().stop();
 // digitalWrite(externalLight, HIGH);
}




void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());  
  Serial.println("Wifi Manager");
  Serial.println("Please connect to AP");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("To setup Wifi Configuration");
//  flashLED(20, 50);
}

void setup() {
  
  //mqtt
  client.setServer(mqtt_server_own, 1883);


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager; 
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();


//set static for device after succesful connect to desired network
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);


  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  wifiManager.setAPCallback(configModeCallback); 
  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect(); 
  //Manual Wifi
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
//    digitalWrite(externalLight, LOW);
    delay(500);
    Serial.print(".");
    cnt++;
 //   digitalWrite(externalLight, HIGH);
  }

/////OTA
   if (ENABLE_OTA) {
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }

//webserver
 if (WEBSERVER_ENABLED) {
    server.on("/", displayHome);
    server.on("/systemreset", handleSystemReset);
    server.on("/forgetwifi", handleWifiReset);
    server.on("/updateconfig", handleUpdateConfig);
    server.on("/configure", handleConfigure);
    server.onNotFound(redirectHome);
    // Start the server
    server.begin();
    Serial.println("Server started");


///saving to fs
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_port, json["mqtt_user"]);
          strcpy(mqtt_port, json["mqtt_pass"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read


//set config save notify callback
wifiManager.setSaveConfigCallback(saveConfigCallback);

//read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());
//save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;
    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
//end save
  }  
 }
}

 void loop() {

    if (ENABLE_OTA) {
    ArduinoOTA.handle();
  }

 if (WEBSERVER_ENABLED) {
    server.handleClient();
  }

  //mqtt
if (!client.connected()) {
    reconnect();
  }
client.loop();
long now = millis();
if (now - lastMsg > 60000) {
lastMsg = now;
++value;
snprintf (msg, 75, "uptime %ld", value);
Serial.print("Publish message: ");
Serial.println(msg);
client.publish("outTopicStatusText196", msg);
//more sysinfos to publish
mqttRSSI( WiFi.RSSI() );
  }
  
}


//mqtt
//mqtt based functions for rssi 
void mqttRSSI(int rssi) {
char stringRSSI[3];
dtostrf(rssi,3, 0, stringRSSI);
char RSSIStrConcat[8];
strcpy( RSSIStrConcat, stringRSSI );
strcat( RSSIStrConcat, "dBm" );
client.publish("outTopicStatusrssi196", RSSIStrConcat);
Serial.print("Publish message: ");
Serial.println(RSSIStrConcat);
}
//mqtt connect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPNodeMCU_196")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopicStatustText196", "hello, is it me you looking for");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}









