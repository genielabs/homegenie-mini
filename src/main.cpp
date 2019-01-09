/*
 * HomeGenie-micro (c) 2018-2019 G-Labs
 *
 * Author:
 * - Generoso Martello <gene@homegenie.it>
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include <Arduino.h>

#include "service/HomeGenie.h"

// method declaration
bool startWPSPBC();

auto homeGenie = Service::HomeGenie();

/// This gets called just before the main application loop()
void setup() {
    // initialization

    // Initialize RX/TX activity LED
    //pinMode(LED_BUILTIN, OUTPUT);
    // ESP D1-mini has LED HIGH/LOW inverted
    //digitalWrite(LED_BUILTIN, HIGH);

    // Enable serial I/O
    Serial.begin(115200);

    // Clear/reset the terminal screen
    // || BEGIN ANSI codes (ESC code)
    Serial.write(27); // ESC
    // clear screen command
    Serial.print("[2J");
    Serial.write(27); // ESC
    // cursor to home command
    Serial.print("[H");
    // \\ END ANSI codes

    // Welcome message
    Serial.println("\n\nHomeGenie Mini V1.0");
    Serial.println("READY.\n");

    Serial.print("\nConfiguring WI-FI...");
    delay(1000);
    startWPSPBC();
    Serial.println("done!\n");

    homeGenie.begin();
}

/// Main application loop
void loop()
{
    homeGenie.loop();
}

//////////////////////////////////////////

bool startWPSPBC() {
    Serial.println("Connecting to WI-FI...");
    // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
    WiFi.mode(WIFI_STA);
    delay(1000);
    // WiFi.begin("foobar",""); // make a failed connection
    WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());
    while (WiFi.status() == WL_DISCONNECTED) {
        delay(1500);
        Serial.print(".");
    }

    bool wpsSuccess = false;
    wl_status_t status = WiFi.status();
    if(status == WL_CONNECTED) {
        Serial.printf("\nConnected successfull to SSID '%s'\n", WiFi.SSID().c_str());
        wpsSuccess = true;
    } else {
        Serial.printf("\nCould not connect to WiFi. state='%d'", status);
        Serial.println("Please press WPS button on your router.\n Press any key to continue...");
        //while(!Serial.available()) { ; }
        //if(!startWPSPBC()) {
        //    Serial.println("Failed to connect with WPS :-(");
        //}
        Serial.println("WPS config start");
        wpsSuccess = WiFi.beginWPSConfig();
        if(wpsSuccess) {
            // Well this means not always success :-/ in case of a timeout we have an empty ssid
            String newSSID = WiFi.SSID();
            if(newSSID.length() > 0) {
                // WPSConfig has already connected in STA mode successfully to the new station.
                Serial.printf("WPS finished. Connected successfull to SSID '%s'", newSSID.c_str());
                // save to config and use next time or just use - WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());
                //qConfig.wifiSSID = newSSID;
                //qConfig.wifiPWD = WiFi.psk();
                //saveConfig();
            } else {
                wpsSuccess = false;
            }
        }

    }

    if (wpsSuccess)
    {
        Serial.println(WiFi.localIP());
    }

    return wpsSuccess;
}
