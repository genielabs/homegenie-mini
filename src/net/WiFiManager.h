//
// Created by gene on 16/02/19.
//
#ifndef HOMEGENIE_MINI_WIFIMANAGER_H
#define HOMEGENIE_MINI_WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <include/wl_definitions.h>

#include <Task.h>
#include <io/Logger.h>

namespace Net {

    class WiFiManager : Task {
    public:
        WiFiManager();
        void loop();
        void initWiFi();
        bool startWPS();
        bool checkWiFiStatus();
    private:
        wl_status_t wiFiStatus;
    };

}
#endif //HOMEGENIE_MINI_WIFIMANAGER_H
