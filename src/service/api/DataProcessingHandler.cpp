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

#include "DataProcessingHandler.h"

#ifndef DISABLE_DATA_PROCESSING

namespace Service { namespace API {

    void DataProcessingHandler::init() {
    }

    bool DataProcessingHandler::canHandleDomain(String* domain) {
        // DataProcessing.Filters/Series/SimpleAverage/HomeAutomation.ZWave/6/Meter.Watts/1732752000000/1732804666110/192
        return domain->equals(IO::IOEventDomains::DataProcessing_Filters);
    }

    bool DataProcessingHandler::handleRequest(Service::APIRequest *request, ResponseCallback* responseCallback) {
        auto homeGenie = HomeGenie::getInstance();
        if (request->Address == "Series") {
            if (request->Command == "SimpleAverage") {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto param = request->getOption(2);
                auto rangeStart = request->getOption(3).toDouble() / 1000.0f; // from ms to s
                auto rangeEnd = request->getOption(4).toDouble() / 1000.0f; // from ms to s
                auto maxWidth = request->getOption(5).toDouble();

                auto step = (rangeEnd - rangeStart) / maxWidth;

                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    auto parameter = module->getProperty(param);
                    if (parameter != nullptr) {
                        responseCallback->beginGetLength();
                        homeGenie->writeParameterHistoryJSON(parameter, responseCallback, 0, STATS_HISTORY_LIMIT, rangeStart, rangeEnd, maxWidth);
                        responseCallback->endGetLength();
                        homeGenie->writeParameterHistoryJSON(parameter, responseCallback, 0, STATS_HISTORY_LIMIT, rangeStart, rangeEnd, maxWidth);
                    }
                }

                return true;
            }
        }
        return false;
    }

    bool DataProcessingHandler::handleEvent(IO::IIOEventSender *sender,
                                   const char* domain, const char* address,
                                   const char *eventPath, void *eventData,
                                   IO::IOEventDataType dataType) {
        return false;
    }

    Module* DataProcessingHandler::getModule(const char* domain, const char* address) {
        return nullptr;
    }
    LinkedList<Module*>* DataProcessingHandler::getModuleList() {
        return &moduleList;
    }

}}

#endif
