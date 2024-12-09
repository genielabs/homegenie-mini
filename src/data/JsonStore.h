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
 */


#ifndef HOMEGENIE_MINI_JSONSTORE_H
#define HOMEGENIE_MINI_JSONSTORE_H

#include <LinkedList.h>
#include <LittleFS.h>

#include "Config.h"

namespace Data {

    template<class T>
    class JsonStore {
    public:
        LinkedList<T*> getItemList(){
            return itemList;
        }
        void load() {
            auto fs = LittleFS;
#ifdef ESP8266
            if(true==fs.begin( )) {
#else
            int maxFiles = 1;
            if(true == fs.begin(true, "/littlefs", maxFiles , "spiffs") && fs.exists(fileName)) {
#endif
                auto f = LittleFS.open(fileName, FILE_READ);
                auto jsonItems = f.readString();
                f.close();
                JsonDocument doc;
                deserializeJson(doc, jsonItems);

                JsonArray array = doc.as<JsonArray>();
                for(JsonVariant v : array) {

                    addItem(getItemFromJson(v));

                }

            } else {

                // TODO: report error

            }
        }
        void save() {
            auto fs = LittleFS;
#ifdef ESP8266
            if(true==fs.begin( )) {
#else
            int maxFiles = 1;
            if(true==fs.begin( true, "/littlefs", maxFiles , "spiffs")) {
#endif
                auto f = LittleFS.open(fileName, FILE_WRITE);
                f.print(getJsonList());
                f.close();

            } else {

                // TODO: report error

            }
        }
        String getJsonList() {
            JsonDocument doc;
            for (int i = 0; i < itemList.size(); i++) {
                auto schedule = itemList.get(i);
                auto jsonItems = doc.add<JsonObject>();
                getJson(&jsonItems, schedule);
            }
            String output;
            serializeJson(doc, output);
            return output;
        }

        virtual void addItem(T* item) = 0;
        virtual T* getItemFromJson(JsonVariant& json) = 0;
        virtual void getJson(JsonObject* jsonItem, T* item) = 0;

    protected:
        LinkedList<T*> itemList;
        const char* fileName;
    };

}


#endif //HOMEGENIE_MINI_JSONSTORE_H
