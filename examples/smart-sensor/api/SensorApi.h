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
 *
 * Releases:
 * - 2025-02-05 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_SENSORAPI_H
#define HOMEGENIE_MINI_SENSORAPI_H

namespace Service { namespace API {

    namespace SensorApi {
        namespace Configuration {

            namespace DHT_Sensor {
                const static char SensorPin[]           = "sdht-pin";
                const static char SensorType[]          = "sdht-typ";
                const static char TemperatureAdjust[]   = "sdht-adj";
            }
            namespace DSB18_Sensor {
                const static char SensorPin[]   = "soth-pin";
                const static char SensorType[]  = "soth-typ";
            }
            namespace Light_Sensor {
                const static char SensorPin[]   = "ligh-pin";
                const static char SensorType[]  = "ligh-typ";
            }
            namespace Color_Sensor {
                const static char SensorType[]  = "colr-typ";
                const static char SDA[]         = "colr-sda";
                const static char SCL[]         = "colr-scl";
            }
            namespace Motion_Sensor {
                const static char SensorType[]  = "motn-typ";
                const static char SensorPin[]   = "motn-pin";
            }
            namespace Camera_Sensor {
                const static char SensorType[]          = "camr-typ";

                const static char CameraPin_xcl[]       = "camr-xcl";
                const static char CameraPin_xfr[]       = "camr-xfr";
                const static char CameraPin_vsn[]       = "camr-vsn";
                const static char CameraPin_hrf[]       = "camr-hrf";
                const static char CameraPin_pcl[]       = "camr-pcl";
                const static char CameraPin_sda[]       = "camr-sda";
                const static char CameraPin_scl[]       = "camr-scl";
                const static char CameraPin_cd0[]       = "camr-cd0";
                const static char CameraPin_cd1[]       = "camr-cd1";
                const static char CameraPin_cd2[]       = "camr-cd2";
                const static char CameraPin_cd3[]       = "camr-cd3";
                const static char CameraPin_cd4[]       = "camr-cd4";
                const static char CameraPin_cd5[]       = "camr-cd5";
                const static char CameraPin_cd6[]       = "camr-cd6";
                const static char CameraPin_cd7[]       = "camr-cd7";
                const static char CameraPin_pwr[]       = "camr-pwr";
                const static char CameraPin_rst[]       = "camr-rst";
                const static char CameraCfg_fbc[]       = "camr-fbc";

                const static char SdCard_clk[]          = "sdmc-clk";
                const static char SdCard_cmd[]          = "sdmc-cmd";
                const static char SdCard_pd0[]          = "sdmc-pd0";
                const static char SdCard_pd1[]          = "sdmc-pd1";
                const static char SdCard_pd2[]          = "sdmc-pd2";
                const static char SdCard_pd3[]          = "sdmc-pd3";

                const static char BrightnessControl[]   = "bri-ctrl";
                const static char SaturationControl[]   = "sat-ctrl";
                const static char ContrastControl[]     = "cnt-ctrl";
                const static char EffectControl[]       = "efx-ctrl";
                const static char HMirrorControl[]      = "h-mirror";
                const static char VFlipControl[]        = "v-flip";
                const static char ImageQuality[]        = "image-qlt";
                const static char ImageResolution[]     = "image-res";
            }

        }
    }
    namespace Options {
        const static char Controls_Brightness[]     = "Controls.Brightness";
        const static char Controls_Saturation[]     = "Controls.Saturation";
        const static char Controls_Contrast[]       = "Controls.Contrast";
        const static char Controls_Effect[]         = "Controls.Effect";
        const static char Controls_HMirror[]        = "Controls.HMirror";
        const static char Controls_VFlip[]          = "Controls.VFlip";
        const static char Image_Quality[]           = "Image.Quality";
        const static char Image_Resolution[]        = "Image.Resolution";
    }

}}

#endif //HOMEGENIE_MINI_SENSORAPI_H
