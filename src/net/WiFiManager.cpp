/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
 *
 *
 * This file is part of HomeGenie-Mini (HGM).
 *
 *  HomeGenie-Mini is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HomeGenie-Mini is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HomeGenie-Mini.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Authors:
 * - Generoso Martello <gene@homegenie.it>
 *
 *
 * Releases:
 * - 2019-02-16 Initial release
 *
 */

#include "WiFiManager.h"

namespace Net {
    wl_status_t WiFiManager::wiFiStatus = WL_NO_SSID_AVAIL;
    unsigned long WiFiManager::wiFiIdleTimestamp = 0;
#ifdef CONFIGURE_WITH_WPS
    bool WiFiManager::temporaryWps = false;
    WpsEventFlags WiFiManager::temporaryWpsEvents{};
#endif
#ifdef ESP32
    esp_wps_config_t WiFiManager::wps_config;
    bool WiFiManager::esp32_wps_started = false;
    unsigned long WiFiManager::esp32_wps_waiting_connect_ts = 0;
#endif

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);
#ifdef CONFIGURE_WITH_WPS
        temporaryWps = Config::isTemporaryConfiguration();
#endif

#ifdef ESP8266
        WiFi.mode(WIFI_STA);
        //WiFi.setOutputPower(20.5);
#else
        WiFiClass::mode(WIFI_STA);
        //WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
        WiFi.setAutoReconnect(true);

#ifdef CONFIGURE_WITH_WPS
#ifdef ESP32
        wps_config.wps_type = ESP_WPS_MODE;
        strcpy(wps_config.factory_info.manufacturer, ESP_WPS_MANUFACTURER);
        strcpy(wps_config.factory_info.model_number, ESP_WPS_MODEL_NUMBER);
        strcpy(wps_config.factory_info.model_name, ESP_WPS_MODEL_NAME);
        strcpy(wps_config.factory_info.device_name, ESP_WPS_DEVICE_NAME);
#endif
        WiFi.onEvent(WiFiEvent);
#endif

        if (Config::isWiFiConfigured()) {
#ifdef ESP8266
            // WI-FI will not boot without this delay!!!
            delay(2000);
#endif
        }
    }

    void WiFiManager::loop() {
#ifdef CONFIGURE_WITH_WPS
        if (temporaryWps) {
            temporaryWpsLoop();
            return;
        }
#endif
        if (!Config::isWiFiConfigured()) return;
        auto status = ESP_WIFI_STATUS;
        if (status != wiFiStatus) {

            // work-around for re-connection issue on version 6.9.0
            if (status != WL_CONNECTED) {
                WiFi.disconnect();
            }

            checkWiFiStatus();
            wiFiStatus = ESP_WIFI_STATUS;
            if (wiFiStatus == WL_DISCONNECTED) {
                wiFiStatus = WL_NO_SSID_AVAIL;
            }
        } else if (wiFiStatus == WL_IDLE_STATUS) {
            if (wiFiIdleTimestamp == 0) {
                wiFiIdleTimestamp = millis();
            } else if (millis() - wiFiIdleTimestamp > 15000) {
                // force reconnect
                wiFiStatus = WL_NO_SSID_AVAIL;
                wiFiIdleTimestamp = 0;
                connect();
            }
        }
    }

    void WiFiManager::connect() {
        IO::Logger::info("|  - Connecting to WI-FI");
        // If stored password is empty use WPS credentials if present
        if (Config::system.pass.isEmpty()) {
            delay(1000); // TODO: is this delay necessary?
#ifdef ESP8266
            // A cancelled WPS may have changed the SDK's current AP. Never
            // substitute that network for the saved HomeGenie configuration.
            if (WiFi.SSID() == Config::system.ssid)
                WiFi.begin(Config::system.ssid.c_str(), WiFi.psk().c_str());
            else WiFi.begin(Config::system.ssid.c_str(), "");
            WiFi.waitForConnectResult(10000);
#else
            wifi_config_t config;
            esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &config);
            if (err == ESP_OK) {

                if (Config::system.ssid == (char*) config.sta.ssid)
                    WiFi.begin(Config::system.ssid.c_str(), (char*) config.sta.password);
                else WiFi.begin(Config::system.ssid.c_str(), "");
                WiFi.waitForConnectResult(10000);

                esp32_wps_waiting_connect_ts = 0;

            } else {
                // TODO: Serial.printf("Couldn't get config: %d\n", (int) err);
            }
#endif
        } else if (!Config::system.ssid.isEmpty() && !Config::system.pass.isEmpty()) {
            IO::Logger::info("|  - WI-FI SSID: \"%s\"", Config::system.ssid.c_str());
            IO::Logger::info("|  - WI-FI Password: ***"); // Config::system.pass.c_str()
            WiFi.begin(Config::system.ssid.c_str(), Config::system.pass.c_str());
            WiFi.waitForConnectResult(10000);
        }
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        auto status = ESP_WIFI_STATUS;
        if (status == WL_CONNECTED) {
            if (status != wiFiStatus) {
                IO::Logger::info("|  - Connected to '%s'", WiFi.SSID().c_str());
                IO::Logger::info("|  - IP: %s", WiFi.localIP().toString().c_str());
            }
            wpsSuccess = true;
        } else {
            switch (status) {
                case WL_NO_SSID_AVAIL:
                    IO::Logger::error("|  x WiFi SSID not available");
                    connect();
                    break;
                case WL_CONNECT_FAILED:
                    IO::Logger::error("|  x WiFi connection failed");
                    connect();
                    break;
                case WL_CONNECTION_LOST:
                    IO::Logger::error("|  x WiFi connection lost");
                    connect();
                    break;
                case WL_DISCONNECTED:
                    IO::Logger::error("|  x WiFi disconnected");
                    connect();
                    break;
                case WL_NO_SHIELD:
                    IO::Logger::error("|  x WiFi initialization failed");
                    break;
                case WL_SCAN_COMPLETED:
                    IO::Logger::error("|  x Not connected to WiFi (state='%d')", status);
                    break;
                case WL_IDLE_STATUS:
                    IO::Logger::error("|  x WiFi idle (?)");
                    break;
            }
        }
        return wpsSuccess;
    }


#ifdef CONFIGURE_WITH_WPS
    bool WiFiManager::startTemporaryWps() {
        // Older WPS configurations kept their password only in the SDK. Copy
        // it into the existing canonical configuration BEFORE starting WPS,
        // so a reset/power interruption cannot lose the previous network.
        String previousSsid, previousPassword;
#ifdef ESP32
        // Recent cores expose SSID() only while associated; read the stored
        // station configuration directly, including at a disconnected boot.
        wifi_config_t station = {};
        if (esp_wifi_get_config(WIFI_IF_STA, &station) == ESP_OK) {
            char ssid[33] = {}, password[65] = {};
            memcpy(ssid, station.sta.ssid, 32);
            memcpy(password, station.sta.password, 64);
            previousSsid = ssid;
            previousPassword = password;
        }
#else
        previousSsid = WiFi.SSID();
        previousPassword = WiFi.psk();
#endif
        if (Config::hasWiFiCredentials() && Config::system.pass.isEmpty() &&
                previousSsid == Config::system.ssid) {
            String password = previousPassword;
            if (!password.isEmpty()) {
                Preferences preferences;
                if (!preferences.begin(CONFIG_SYSTEM_NAME, false)) return false;
                preferences.putString(CONFIG_KEY_wifi_password, password);
                bool saved = preferences.getString(CONFIG_KEY_wifi_password, "") == password;
                preferences.end();
                if (!saved) return false;
                Config::system.pass = password;
            }
        }
#ifdef ESP32
        // WPS credentials stay in RAM until a new connection obtains an IP.
        if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) return false;
#else
        disable_extra4k_at_link_time(); // Same WPS stack requirement as the Arduino core.
        WiFi.persistent(false);
#endif
        WiFi.setAutoReconnect(false);
        WiFi.disconnect();
        temporaryWpsEvents.store(0);
        IO::Logger::info("|  >> Press WPS button on your router <<");
#ifdef ESP32
        return wpsStart();
#else
        wifi_wps_disable();
        return wifi_wps_enable(WPS_TYPE_PBC) &&
            wifi_set_wps_cb(temporaryWpsCallback) && wifi_wps_start();
#endif
    }

    void WiFiManager::stopTemporaryWps() {
#ifdef ESP32
        wpsStop();
#else
        wifi_wps_disable();
#endif
    }

    bool WiFiManager::saveTemporaryWpsCredentials() {
        const String ssid = WiFi.SSID();
        const String password = WiFi.psk();
        if (ssid.isEmpty()) return false;
        Preferences preferences;
        if (!preferences.begin(CONFIG_SYSTEM_NAME, false)) return false;
        const String previousSsid = preferences.getString(CONFIG_KEY_wifi_ssid, "");
        const String previousPassword = preferences.getString(CONFIG_KEY_wifi_password, "");
        preferences.putString(CONFIG_KEY_wifi_password, password);
        preferences.putString(CONFIG_KEY_wifi_ssid, ssid);
        bool saved = preferences.getString(CONFIG_KEY_wifi_ssid, "") == ssid &&
            preferences.getString(CONFIG_KEY_wifi_password, "") == password;
        if (!saved) {
            preferences.putString(CONFIG_KEY_wifi_password, previousPassword);
            preferences.putString(CONFIG_KEY_wifi_ssid, previousSsid);
        }
        preferences.end();
        if (!saved) return false;
        Config::system.ssid = ssid;
        Config::system.pass = password;
        Config::onWiFiConfigured();
        return true;
    }

    void WiFiManager::temporaryWpsLoop() {
        const unsigned long now = millis();
        if (temporaryWpsPhase == WpsPhase::Initial) {
            if (startTemporaryWps()) {
                temporaryWpsPhase = WpsPhase::Registering;
                temporaryWpsTimestamp = now;
            } else {
                stopTemporaryWps();
                temporaryWpsPhase = WpsPhase::Finished;
                IO::Logger::error("|  x Could not start WPS");
            }
            return;
        }
        const unsigned events = temporaryWpsEvents.exchange(0);
        if (temporaryWpsPhase == WpsPhase::Registering) {
            if ((events & 2u) || now - temporaryWpsTimestamp >= 40000) {
                stopTemporaryWps();
                temporaryWpsPhase = WpsPhase::Finished;
                IO::Logger::info("|  - WPS ended without a new connection");
            } else if (events & 1u) {
                stopTemporaryWps();
                temporaryWpsPhase = WpsPhase::Associating;
                temporaryWpsTimestamp = now;
            }
        } else if (temporaryWpsPhase == WpsPhase::Associating) {
            if (!temporaryWpsConnectRequested && now - temporaryWpsTimestamp >= 2000) {
                // Retain the original settling delay, without blocking the
                // event task or reconnecting with the OLD Config password.
                temporaryWpsConnectRequested = true;
#ifdef ESP32
                esp_wifi_connect();
#else
                wifi_station_connect();
#endif
            } else if (temporaryWpsConnectRequested && (events & 4u) && WiFi.isConnected()) {
                if (saveTemporaryWpsCredentials()) {
                    temporaryWpsPhase = WpsPhase::Restart;
                    temporaryWpsTimestamp = now;
                    IO::Logger::info("|  - WPS configured; restarting");
                } else {
                    temporaryWpsPhase = WpsPhase::Finished;
                    IO::Logger::error("|  x Could not save WPS credentials");
                }
            } else if (now - temporaryWpsTimestamp >= 17000) {
                WiFi.disconnect();
                temporaryWpsPhase = WpsPhase::Finished;
                IO::Logger::info("|  - WPS connection timed out");
            }
        } else if (temporaryWpsPhase == WpsPhase::Restart && now - temporaryWpsTimestamp >= 1000) {
            ESP.restart();
        }
    }
#endif

    bool WiFiManager::configure(bool rebootIntoConfiguration) {
#ifdef CONFIGURE_WITH_WPS
        // Serial #WPS during temporary setup remains a credential reset,
        // restarting the same BOOT 2 flow rather than mixing WPS handlers.
        if (temporaryWps) rebootIntoConfiguration = true;
#endif
        Preferences preferences;
        if (!preferences.begin(CONFIG_SYSTEM_NAME, false)) return false;
        preferences.putString(CONFIG_KEY_wifi_ssid, "");
        preferences.putString(CONFIG_KEY_wifi_password, "");
        preferences.end();
#ifdef ESP8266
        ESP.eraseConfig();
#else
        // TODO: find a way to clear wi-fi credentials on ESP32 the
        //       following commented code was apparently not working
//        wifi_config_t wifi_cfg_empty;
//        memset(&wifi_cfg_empty, 0, sizeof(wifi_config_t));
//        esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg_empty);
#endif
        IO::Logger::info("|  - WI-FI config reset!");
        if (rebootIntoConfiguration) {
            // BOOT 2 erases credentials, then starts WPS on the temporary boot.
#ifdef ESP32
#ifdef CONFIGURE_WITH_WPS
            // BOOT 1 uses RAM storage. BOOT 2 must erase the FLASH copy too,
            // otherwise the SDK could reuse the previous AP after a reboot.
            if (esp_wifi_set_storage(WIFI_STORAGE_FLASH) != ESP_OK) return false;
#endif
            WiFi.disconnect(false, true);
#else
            WiFi.disconnect();
#endif
            return Config::restartInConfigurationMode(true);
        }
#ifdef CONFIGURE_WITH_WPS
        Config::statusLedOff();
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        Config::statusLedOn();
        IO::Logger::info ("|  >> Press WPS button on your router <<");
#ifdef ESP8266
        return WiFi.beginWPSConfig();
#else
        int timeout = 400;
        bool success = wpsStart();
        while (esp32_wps_started && timeout-- > 0) {
            delay(100);
            yield();
        }
        wpsStop();
        return success;
#endif
#else
        delay(2000);
        IO::Logger::info("RESTART!");
        // Restart device
        ESP.restart();
        return true;
#endif
    }

}