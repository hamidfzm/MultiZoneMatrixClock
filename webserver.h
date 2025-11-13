#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESP8266WebServer.h>
#include "config.h"
#include "timezone.h"
#include "display.h"

class WebServerManager {
public:
  WebServerManager(ESP8266WebServer* srv, TimezoneManager* tzm, Settings* sett, DisplayManager* disp);
  void begin();
  void handleClient();
  
private:
  void handleRoot();
  void handleSave();
  void handleWifiPortal();
  String generateHTML();
  
  ESP8266WebServer* server;
  TimezoneManager* tzManager;
  Settings* settings;
  DisplayManager* displayManager;
};

#endif

