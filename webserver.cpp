#include "webserver.h"
#include <WiFiManager.h>
#include <EEPROM.h>
#include "config.h"

WebServerManager::WebServerManager(ESP8266WebServer* srv, TimezoneManager* tzm, Settings* sett, DisplayManager* disp) {
  server = srv;
  tzManager = tzm;
  settings = sett;
  displayManager = disp;
}

void WebServerManager::begin() {
  server->on("/", [this]() { this->handleRoot(); });
  server->on("/save", HTTP_POST, [this]() { this->handleSave(); });
  server->on("/wifi", HTTP_POST, [this]() { this->handleWifiPortal(); });
  server->begin();
}

void WebServerManager::handleClient() {
  server->handleClient();
}

String WebServerManager::generateHTML() {
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Multi-Zone Matrix Clock</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 40px;
            max-width: 600px;
            width: 100%;
            animation: slideIn 0.5s ease-out;
        }
        
        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-20px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        h1 {
            color: #667eea;
            margin-bottom: 30px;
            text-align: center;
            font-size: 2em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
        }
        
        .section {
            margin-bottom: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
            border-left: 4px solid #667eea;
        }
        
        .section-title {
            font-size: 1.2em;
            font-weight: bold;
            color: #333;
            margin-bottom: 15px;
            display: flex;
            align-items: center;
        }
        
        .section-title::before {
            content: "⚙️";
            margin-right: 10px;
        }
        
        .checkbox-group {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-top: 10px;
        }
        
        .checkbox-item {
            display: flex;
            align-items: center;
            padding: 10px;
            background: white;
            border-radius: 8px;
            transition: all 0.3s ease;
            cursor: pointer;
        }
        
        .checkbox-item:hover {
            background: #e9ecef;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        
        .checkbox-item input[type="checkbox"] {
            width: 20px;
            height: 20px;
            margin-right: 10px;
            cursor: pointer;
            accent-color: #667eea;
        }
        
        .checkbox-item label {
            cursor: pointer;
            font-size: 1em;
            color: #333;
            flex: 1;
        }
        
        .input-group {
            margin-top: 15px;
        }
        
        .input-group label {
            display: block;
            margin-bottom: 8px;
            color: #555;
            font-weight: 500;
        }
        
        .input-group input[type="number"] {
            width: 100%;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 1em;
            transition: border-color 0.3s;
        }
        
        .input-group input[type="number"]:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .radio-group {
            display: flex;
            gap: 20px;
            margin-top: 10px;
        }
        
        .radio-item {
            display: flex;
            align-items: center;
            padding: 12px 20px;
            background: white;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            border: 2px solid #ddd;
        }
        
        .radio-item:hover {
            background: #f0f0f0;
            border-color: #667eea;
        }
        
        .radio-item input[type="radio"] {
            margin-right: 8px;
            cursor: pointer;
            accent-color: #667eea;
        }
        
        .radio-item label {
            cursor: pointer;
            color: #333;
        }
        
        .radio-item input[type="radio"]:checked + label {
            color: #667eea;
            font-weight: bold;
        }
        
        .btn {
            padding: 15px 30px;
            border: none;
            border-radius: 8px;
            font-size: 1.1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            width: 100%;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
        }
        
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
        }
        
        .btn-primary:active {
            transform: translateY(0);
        }
        
        .btn-secondary {
            background: #6c757d;
            color: white;
            width: 100%;
            margin-top: 15px;
        }
        
        .btn-secondary:hover {
            background: #5a6268;
            transform: translateY(-2px);
        }
        
        form {
            margin-bottom: 20px;
        }
        
        .status {
            text-align: center;
            padding: 15px;
            margin-top: 20px;
            border-radius: 8px;
            background: #d4edda;
            color: #155724;
            display: none;
        }
        
        @media (max-width: 600px) {
            .container {
                padding: 20px;
            }
            
            .checkbox-group {
                grid-template-columns: 1fr;
            }
            
            .radio-group {
                flex-direction: column;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌍 Multi-Zone Matrix Clock</h1>
        
        <form method="POST" action="/save">
            <div class="section">
                <div class="section-title">Timezones</div>
                <div class="checkbox-group">
)";

  // Add timezone checkboxes
  for (int i = 0; i < TZ_COUNT; i++) {
    const char* tzName = tzManager->getTimezoneName(i);
    if (strlen(tzName) > 0) {
      html += "                    <div class=\"checkbox-item\">\n";
      html += "                        <input type=\"checkbox\" id=\"tz" + String(i) + "\" name=\"tz" + String(i) + "\"";
      if (tzManager->tzEnabled[i]) html += " checked";
      html += ">\n";
      html += "                        <label for=\"tz" + String(i) + "\">" + String(tzName) + "</label>\n";
      html += "                    </div>\n";
    }
  }

  html += R"(
                </div>
            </div>
            
            <div class="section">
                <div class="section-title">Display Settings</div>
                <div class="input-group">
                    <label for="intensity">Display Intensity (0-15)</label>
                    <input type="number" id="intensity" name="intensity" min="0" max="15" value=")";
  html += String(settings->intensity);
  html += R"(" required>
                </div>
                <div class="input-group">
                    <label for="timeDisplayDuration">Time Display Duration (seconds, 1-60)</label>
                    <input type="number" id="timeDisplayDuration" name="timeDisplayDuration" min="1" max="60" value=")";
  html += String(settings->timeDisplayDuration);
  html += R"(" required>
                </div>
            </div>
            
            <div class="section">
                <div class="section-title">Debug Settings</div>
                <div class="checkbox-item">
                    <input type="checkbox" id="debug" name="debug")";
  if (settings->debugEnabled) html += " checked";
  html += R"(>
                    <label for="debug">Enable Debug Logging</label>
                </div>
            </div>
            
            <div class="section">
                <div class="section-title">Time Format</div>
                <div class="radio-group">
                    <div class="radio-item">
                        <input type="radio" id="format24" name="format" value="24")";
  if (!settings->use12Hour) html += " checked";
  html += R"(>
                        <label for="format24">24-Hour</label>
                    </div>
                    <div class="radio-item">
                        <input type="radio" id="format12" name="format" value="12")";
  if (settings->use12Hour) html += " checked";
  html += R"(>
                        <label for="format12">12-Hour (AM/PM)</label>
                    </div>
                </div>
            </div>
            
            <button type="submit" class="btn btn-primary">💾 Save Settings</button>
        </form>
        
        <form method="POST" action="/wifi">
            <button type="submit" class="btn btn-secondary">📶 WiFi Configuration</button>
        </form>
    </div>
</body>
</html>
)";

  return html;
}

void WebServerManager::handleRoot() {
  server->send(200, "text/html", generateHTML());
}

void WebServerManager::handleSave() {
  // Update timezone enabled states
  for (int i = 0; i < TZ_COUNT; i++) {
    tzManager->tzEnabled[i] = server->hasArg("tz" + String(i));
  }
  
  // Update intensity
  if (server->hasArg("intensity")) {
    settings->intensity = server->arg("intensity").toInt();
    if (settings->intensity > 15) settings->intensity = 15;
    if (settings->intensity < 0) settings->intensity = 0;
    displayManager->setIntensity(settings->intensity);
  }
  
  // Update time format
  settings->use12Hour = server->hasArg("format") && server->arg("format") == "12" ? 1 : 0;
  
  // Update debug flag
  settings->debugEnabled = server->hasArg("debug") ? 1 : 0;
  
  // Update time display duration
  if (server->hasArg("timeDisplayDuration")) {
    int duration = server->arg("timeDisplayDuration").toInt();
    if (duration < 1) duration = 1;
    if (duration > 60) duration = 60;
    settings->timeDisplayDuration = duration;
    displayManager->setTimeDisplayDuration(duration * 1000); // Convert to milliseconds
  }
  
  // Save to EEPROM
  settings->magic = EEPROM_MAGIC;
  for (int i = 0; i < TZ_COUNT; i++) {
    settings->enabled[i] = tzManager->tzEnabled[i] ? 1 : 0;
  }
  EEPROM.put(0, *settings);
  EEPROM.commit();
  
  // Send response immediately to prevent hanging
  server->sendHeader("Location", "/");
  server->send(302, "text/plain", "");
  server->client().stop(); // Close connection immediately
}

void WebServerManager::handleWifiPortal() {
  // Send response first to prevent hanging
  server->send(200, "text/html", "<html><body><h1>WiFi Configuration Portal Starting...</h1><p>Please connect to the 'ESP8266-Config' network and configure WiFi.</p><p>This page will close automatically.</p><script>setTimeout(function(){window.location.href='/';}, 3000);</script></body></html>");
  server->client().stop(); // Close connection immediately
  
  // Start WiFi portal in a non-blocking way
  // Note: This will still block, but at least we've sent the response
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  wm.startConfigPortal("ESP8266-Config");
}

