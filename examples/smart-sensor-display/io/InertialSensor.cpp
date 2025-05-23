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


#include "InertialSensor.h"

namespace IO { namespace Sensors {

    void InertialSensor::begin() {
        bool ret = false;

        auto sda = Config::getSetting("imus-sda").toInt();
        auto scl = Config::getSetting("imus-scl").toInt();
        ret = qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, sda, scl);

        if (!ret) {
            Serial.println("Failed to find QMI8658 - check your wiring!");
        } else {

            /* Get chip id*/
            Serial.print("Device ID:");
            Serial.println(qmi.getChipID(), HEX);


            if (qmi.selfTestAccel()) {
                Serial.println("Accelerometer self-test successful");
            } else {
                Serial.println("Accelerometer self-test failed!");
            }

            if (qmi.selfTestGyro()) {
                Serial.println("Gyroscope self-test successful");
            } else {
                Serial.println("Gyroscope self-test failed!");
            }

            qmi.configAccelerometer(
                    /*
                     * ACC_RANGE_2G
                     * ACC_RANGE_4G
                     * ACC_RANGE_8G
                     * ACC_RANGE_16G
                     * */
                    SensorQMI8658::ACC_RANGE_4G,
                    /*
                     * ACC_ODR_1000H
                     * ACC_ODR_500Hz
                     * ACC_ODR_250Hz
                     * ACC_ODR_125Hz
                     * ACC_ODR_62_5Hz
                     * ACC_ODR_31_25Hz
                     * ACC_ODR_LOWPOWER_128Hz
                     * ACC_ODR_LOWPOWER_21Hz
                     * ACC_ODR_LOWPOWER_11Hz
                     * ACC_ODR_LOWPOWER_3H
                    * */
                    SensorQMI8658::ACC_ODR_1000Hz,
                    /*
                    *  LPF_MODE_0     //2.66% of ODR
                    *  LPF_MODE_1     //3.63% of ODR
                    *  LPF_MODE_2     //5.39% of ODR
                    *  LPF_MODE_3     //13.37% of ODR
                    *  LPF_OFF        // OFF Low-Pass Fitter
                    * */
                    SensorQMI8658::LPF_MODE_0);

            qmi.configGyroscope(
                    /*
                    * GYR_RANGE_16DPS
                    * GYR_RANGE_32DPS
                    * GYR_RANGE_64DPS
                    * GYR_RANGE_128DPS
                    * GYR_RANGE_256DPS
                    * GYR_RANGE_512DPS
                    * GYR_RANGE_1024DPS
                    * */
                    SensorQMI8658::GYR_RANGE_64DPS,
                    /*
                     * GYR_ODR_7174_4Hz
                     * GYR_ODR_3587_2Hz
                     * GYR_ODR_1793_6Hz
                     * GYR_ODR_896_8Hz
                     * GYR_ODR_448_4Hz
                     * GYR_ODR_224_2Hz
                     * GYR_ODR_112_1Hz
                     * GYR_ODR_56_05Hz
                     * GYR_ODR_28_025H
                     * */
                    SensorQMI8658::GYR_ODR_896_8Hz,
                    /*
                    *  LPF_MODE_0     //2.66% of ODR
                    *  LPF_MODE_1     //3.63% of ODR
                    *  LPF_MODE_2     //5.39% of ODR
                    *  LPF_MODE_3     //13.37% of ODR
                    *  LPF_OFF        // OFF Low-Pass Fitter
                    * */
                    SensorQMI8658::LPF_MODE_3);

            /*
            * If both the accelerometer and gyroscope sensors are turned on at the same time,
            * the output frequency will be based on the gyroscope output frequency.
            * The example configuration is 896.8HZ output frequency,
            * so the acceleration output frequency is also limited to 896.8HZ
            * */
            qmi.enableGyroscope();
            qmi.enableAccelerometer();

            // Print register configuration information
            qmi.dumpCtrlRegister();

            initialized = true;
        }
    }

    void InertialSensor::loop() {
        // When the interrupt pin is passed in through setPin,
        // the GPIO will be read to see if the data is ready.
        if (initialized && qmi.getDataReady()) {
            /*
            // Serial.print("Timestamp:");
            // Serial.print(qmi.getTimestamp());
            if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
                // Print to serial plotter
                Serial.print("ACCEL.x:"); Serial.print(acc.x); Serial.print(",");
                Serial.print("ACCEL.y:"); Serial.print(acc.y); Serial.print(",");
                Serial.print("ACCEL.z:"); Serial.print(acc.z); Serial.println();
                // m2/s to mg
                Serial.print(" ACCEL.x:"); Serial.print(acc.x * 1000); Serial.println(" mg");
                Serial.print(",ACCEL.y:"); Serial.print(acc.y * 1000); Serial.println(" mg");
                Serial.print(",ACCEL.z:"); Serial.print(acc.z * 1000); Serial.println(" mg");
            }
            if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
                // Print to serial plotter
                Serial.print("GYRO.x:"); Serial.print(gyr.x); Serial.print(",");
                Serial.print("GYRO.y:"); Serial.print(gyr.y); Serial.print(",");
                Serial.print("GYRO.z:"); Serial.print(gyr.z); Serial.println();
                // Serial.print(" GYRO.x:"); Serial.print(gyr.x); Serial.println(" degrees/sec");
                // Serial.print(",GYRO.y:"); Serial.print(gyr.y); Serial.println(" degrees/sec");
                // Serial.print(",GYRO.z:"); Serial.print(gyr.z); Serial.println(" degrees/sec");
            }
*/
            if (millis() - lastTemperatureReadTs > TEMPERATURE_READ_INTERVAL_MS) {
                auto t = Utility::roundDecimals(qmi.getTemperature_C());
                if (std::fabs(temperature - t) > .5) {
                    temperature = t;
                    //Serial.print("Temperature:");
                    //Serial.print(temperature);
                    //Serial.println(" degrees C");
                    // TODO: emit Sensor.IMU.Temperature value (?)
                }
                lastTemperatureReadTs = millis();
            }

            determine_screen_rotation();
        }
    }

    void InertialSensor::set_actual_screen_rotation(InertialSensor::ScreenOrientation new_orientation) {
        if (new_orientation != last_reported_orientation) {
            last_reported_orientation = new_orientation;
            String orientationText;
            switch (new_orientation) {
                case ORIENTATION_PORTRAIT:          orientationText = "PORTRAIT"; break;
                case ORIENTATION_LANDSCAPE_RIGHT:   orientationText = "LANDSCAPE_RIGHT"; break;
                case ORIENTATION_PORTRAIT_INVERTED: orientationText = "PORTRAIT_INVERTED"; break;
                case ORIENTATION_LANDSCAPE_LEFT:    orientationText = "LANDSCAPE_LEFT"; break;
                case ORIENTATION_FLAT_FACE_UP:      orientationText = "FLAT_FACE_UP"; break;
                case ORIENTATION_FLAT_FACE_DOWN:    orientationText = "FLAT_FACE_DOWN"; break;
                default:                            orientationText = "UNKNOWN"; break;
            }
            Logger::info("@%s [%s %s]", INERTIAL_SENSOR_NS_PREFIX, IOEventPaths::Sensor_Orientation, orientationText.c_str());
            sendEvent(IOEventPaths::Sensor_Orientation, &orientationText, IOEventDataType::Text);
            if (orientationChangeCallback != nullptr) {
                orientationChangeCallback(new_orientation);
            }
        }
    }

    void InertialSensor::determine_screen_rotation() {
        bool is_moving = false;

        // Attempt to read gyroscope data
        // Assumes qmi.getGyroscope populates the global 'gyr' and returns true on success.
        if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
            float total_angular_velocity = sqrtf(gyr.x * gyr.x + gyr.y * gyr.y + gyr.z * gyr.z);
            if (total_angular_velocity > GYRO_MOTION_THRESHOLD_DPS) {
                is_moving = true;
            }
        } else {
            // Serial.println("Warning: Failed to read gyroscope data.");
            // Decide on fallback: e.g., assume not moving, or skip orientation change if gyro fails.
        }

        // Attempt to read accelerometer data
        // Assumes qmi.getAccelerometer populates the global 'acc' and returns true on success.
        if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
            ScreenOrientation detected_orientation = ORIENTATION_UNKNOWN;
            // Check if device is relatively flat (Z-axis is dominant)
            if (fabsf(acc.z) > ACCEL_NORMALIZED_THRESHOLD_STRONG) {
                if (acc.z < -ACCEL_NORMALIZED_THRESHOLD_WEAK) { // acc.z is strongly negative (e.g., -0.7 to -1.0)
                    detected_orientation = ORIENTATION_FLAT_FACE_UP;
                } else if (acc.z > ACCEL_NORMALIZED_THRESHOLD_WEAK) { // acc.z is strongly positive (e.g., +0.7 to +1.0)
                    detected_orientation = ORIENTATION_FLAT_FACE_DOWN;
                }
            }
            // (This assumes your 'Y' axis now corresponds to what was previously 'X' for landscape)
            else if (fabsf(acc.y) > ACCEL_NORMALIZED_THRESHOLD_STRONG && fabsf(acc.x) < ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                // If acc.y < -THRESHOLD now means LANDSCAPE_RIGHT
                if (acc.y < -ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                    detected_orientation = ORIENTATION_LANDSCAPE_RIGHT;
                } else if (acc.y > ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                    detected_orientation = ORIENTATION_LANDSCAPE_LEFT;
                }
            }
            // (This assumes your 'X' axis now corresponds to what was previously 'Y' for portrait)
            else if (fabsf(acc.x) > ACCEL_NORMALIZED_THRESHOLD_STRONG && fabsf(acc.y) < ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                // If acc.x < -THRESHOLD now means PORTRAIT_INVERTED
                if (acc.x < -ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                    detected_orientation = ORIENTATION_PORTRAIT_INVERTED;
                } else if (acc.x > ACCEL_NORMALIZED_THRESHOLD_WEAK) {
                    detected_orientation = ORIENTATION_PORTRAIT;
                }
            }

            // Stability and hysteresis logic
            if (is_moving) {
                // If device is moving, reset the stability timer for the candidate.
                // This prevents orientation changes during active motion.
                last_orientation_change_time = millis();
                // Optionally, reset candidate to current to prevent "remembering" a fleeting orientation
                // candidate_orientation = current_orientation;
            } else {
                // If a new, valid orientation is detected and it's different from the current candidate
                if (detected_orientation != ORIENTATION_UNKNOWN && detected_orientation != candidate_orientation) {
                    candidate_orientation = detected_orientation;
                    last_orientation_change_time = millis(); // Start timing for this new candidate
                }

                // If the candidate is valid, different from the current committed orientation,
                // and has been stable for the required duration:
                if (candidate_orientation != ORIENTATION_UNKNOWN &&
                    candidate_orientation != current_orientation &&
                    (millis() - last_orientation_change_time > ORIENTATION_STABILITY_DELAY_MS)) {

                    current_orientation = candidate_orientation;
                    set_actual_screen_rotation(current_orientation);
                }
            }
        } else {
            // Serial.println("Warning: Failed to read accelerometer data.");
        }
    }
}}