/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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


#ifndef HOMEGENIE_MINI_INERTIALSENSOR_H
#define HOMEGENIE_MINI_INERTIALSENSOR_H

#include <HomeGenie.h>

#include <SensorQMI8658.hpp>

#define INERTIAL_SENSOR_NS_PREFIX           "IO::Sensors::InertialSensor"

namespace IO { namespace Sensors {

    class InertialSensor: Task, public IIOEventSender {
    public:
        typedef enum {
            ORIENTATION_PORTRAIT,
            ORIENTATION_LANDSCAPE_LEFT,    // Left edge down
            ORIENTATION_PORTRAIT_INVERTED,
            ORIENTATION_LANDSCAPE_RIGHT,   // Right edge down
            ORIENTATION_FLAT_FACE_UP,      // Flat with screen facing up
            ORIENTATION_FLAT_FACE_DOWN,    // Flat with screen facing down
            ORIENTATION_UNKNOWN
        } ScreenOrientation;

        InertialSensor() {
            setLoopInterval(100); // update every 100 ms
        }

        void begin() override;
        void loop() override;

        void onOrientationChange(std::function<void(ScreenOrientation)> callback) {
            orientationChangeCallback = std::move(callback);
        }
    private:
        SensorQMI8658 qmi;
        IMUdata acc{};
        IMUdata gyr{};

        bool initialized = false;

        float temperature = 0;
        unsigned long lastTemperatureReadTs = 0;
        const uint16_t TEMPERATURE_READ_INTERVAL_MS = 30000;

        ScreenOrientation current_orientation = ORIENTATION_UNKNOWN;
        ScreenOrientation last_reported_orientation = ORIENTATION_UNKNOWN;
        ScreenOrientation candidate_orientation = ORIENTATION_UNKNOWN;

        unsigned long last_orientation_change_time = 0; // For debounce/hysteresis
        unsigned long orientation_check_last_ts = 0;    // For timing the orientation check

        const float ACCEL_NORMALIZED_THRESHOLD_STRONG = 0.70f; // Min fraction of g on an axis to be considered dominant
        const float ACCEL_NORMALIZED_THRESHOLD_WEAK = 0.30f;   // Max fraction of g on other axes for a clear orientation
        // Also used as a dead-zone around 0 for sign checks.

        // For orientation debouncing/hysteresis
        const unsigned long ORIENTATION_STABILITY_DELAY_MS = 500; // Must remain stable for 500ms

        // For gyroscope (to detect if the device is moving)
        const float GYRO_MOTION_THRESHOLD_DPS = 20.0f; // Degrees per second

        void set_actual_screen_rotation(ScreenOrientation new_orientation);
        void determine_screen_rotation();

        std::function<void(ScreenOrientation)> orientationChangeCallback = nullptr;
    };

}}

#endif //HOMEGENIE_MINI_INERTIALSENSOR_H
