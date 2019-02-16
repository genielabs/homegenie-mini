//
// Created by gene on 16/02/19.
//

#include <Config.h>
#include "WiFiManager.h"

namespace Net {

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);
        wiFiStatus = WL_DISCONNECTED;
        // WI-FI will not boot without this delay!!!
        delay(2000);
        initWiFi();
    }

    void WiFiManager::loop() {
        wl_status_t status = WiFi.status();
        if (status != wiFiStatus) {
            wiFiStatus = status;
            checkWiFiStatus();
        }
    }

    void WiFiManager::initWiFi() {
        IO::Logger::info("|  - Connecting to WI-FI .");
        // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
        WiFi.mode(WIFI_STA);
        delay(1000); // TODO: is this delay necessary?
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED) {
            digitalWrite(Config::StatusLedPin, LOW);
            IO::Logger::info("|  ✔ Connected to '%s'", WiFi.SSID().c_str());
            IO::Logger::info("|  ✔ IP: %s", WiFi.localIP().toString().c_str());
            wpsSuccess = true;
        } else if (status == WL_CONNECTION_LOST || status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED) {
            digitalWrite(Config::StatusLedPin, HIGH);
            IO::Logger::error("|  x Lost connection to WiFi");
            initWiFi();
        } else {
            digitalWrite(Config::StatusLedPin, HIGH);
            IO::Logger::error("|  x Not connected to WiFi (state='%d')", status);
        }
        return wpsSuccess;
    }

    bool WiFiManager::startWPS() {
        digitalWrite(Config::StatusLedPin, LOW);
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        digitalWrite(Config::StatusLedPin, HIGH);
        IO::Logger::info ("|  >> Press WPS button on your router <<");
        bool wpsSuccess = WiFi.beginWPSConfig();
        return wpsSuccess;
    }

}