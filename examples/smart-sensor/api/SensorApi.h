//
// Created by gene on 05/02/25.
//

#ifndef HOMEGENIE_MINI_SENSORAPI_H
#define HOMEGENIE_MINI_SENSORAPI_H

namespace Service { namespace API {

    namespace SensorApi {
        namespace Configuration {

            namespace DHT_Sensor {
                const static char SensorPin[] = "sdht-pin";
                const static char SensorType[] = "sdht-typ";
                const static char TemperatureAdjust[] = "sdht-adj";
            }
            namespace DSB18_Sensor {
                const static char SensorPin[] = "soth-pin";
                const static char SensorType[] = "soth-typ";
            }
            namespace Light_Sensor {
                const static char SensorPin[] = "ligh-pin";
                const static char SensorType[] = "ligh-typ";
            }
            namespace Color_Sensor {
                const static char SensorType[] = "colr-typ";
                const static char SDA[] = "colr-sda";
                const static char SCL[] = "colr-scl";
            }
            namespace Motion_Sensor {
                const static char SensorType[] = "motn-typ";
                const static char SensorPin[] = "motn-pin";
            }

        }
    }

}}

#endif //HOMEGENIE_MINI_SENSORAPI_H
