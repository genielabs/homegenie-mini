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

#include "LevelControlActivity.h"

#ifdef ENABLE_UI
#ifndef DISABLE_LVGL

namespace UI { namespace Activities { namespace Control {
    static lv_style_t style_static_bg;

    void LevelControlActivity::attach(LGFX_Device *displayDevice) {
        Activity::attach(displayDevice);
        LV_FONT_DECLARE(material_symbols_48);
    }
    void LevelControlActivity::onResume() {
#ifdef BOARD_HAS_PSRAM
        canvas->setColorDepth(lgfx::rgb565_2Byte);
#else
        canvas->setColorDepth(lgfx::rgb332_1Byte);
#endif
        LvglDriver::begin(canvas);
        activeScreen = lv_scr_act();
        auto canvasWidth = canvas->width();

        // Background gradient
        lv_style_init(&style_static_bg);
        lv_style_set_radius(&style_static_bg, 0);
        lv_style_set_border_width(&style_static_bg, 0);
        lv_style_set_bg_opa(&style_static_bg, LV_OPA_COVER);
        lv_style_set_bg_grad_dir(&style_static_bg, LV_GRAD_DIR_VER);
        lv_style_set_bg_color(&style_static_bg, lv_color_make(20, 30, 130));
        lv_style_set_bg_grad_color(&style_static_bg, lv_color_make(130, 40, 110));
        lv_obj_add_style(activeScreen, &style_static_bg, 0);

        if (!ledStatus) {
            // Status LED
            ledStatus = lv_led_create(activeScreen);
            lv_obj_align(ledStatus, LV_ALIGN_CENTER, 0, 60);
            lv_led_set_color(ledStatus, lv_palette_main(LV_PALETTE_LIME));
            lv_obj_set_size(ledStatus, 12, 12);
            applyCommonStyles(ledStatus);
            if (data.level > 0) {
                lv_led_on(ledStatus);
            } else {
                lv_led_off(ledStatus);
            }
        }

        if (!sliderLevel) {
            // Level slider
            sliderLevel = lv_slider_create(activeScreen);
            lv_obj_set_size(sliderLevel, canvasWidth - 60, 24);
            lv_obj_align(sliderLevel, LV_ALIGN_CENTER, 0, -10);
            lv_obj_add_flag(sliderLevel, LV_OBJ_FLAG_CLICKABLE);
            lv_slider_set_value(sliderLevel, data.level, LV_ANIM_OFF);
            applyCommonStyles(sliderLevel);
            // Slider label
            labelLevel = lv_label_create(activeScreen);
            auto l = String(data.level); l.concat('%');
            lv_label_set_text(labelLevel, l.c_str());
            lv_obj_align_to(labelLevel, sliderLevel, LV_ALIGN_OUT_BOTTOM_MID, 0, -20);
            lv_obj_add_event_cb(sliderLevel, sliderEventHandler, LV_EVENT_VALUE_CHANGED, this);
            activityEventsLock(sliderLevel, this);
        }

        if (!buttonOn) {
            buttonOn = lv_btn_create(activeScreen);
            lv_obj_align(buttonOn, LV_ALIGN_CENTER, (canvasWidth / 4.5f), 60);
            lv_obj_set_size(buttonOn, 60, 50);
            applyCommonStyles(buttonOn);
            labelOn = lv_label_create(buttonOn);
            lv_label_set_text(labelOn, "ON");
            lv_obj_center(labelOn);
            lv_obj_add_event_cb(buttonOn, buttonEventHandler, LV_EVENT_CLICKED, this);
            activityEventsLock(buttonOn, this);
        }

        if (!buttonOff) {
            buttonOff = lv_btn_create(activeScreen);
            lv_obj_align(buttonOff, LV_ALIGN_CENTER, -(canvasWidth / 4.5f), 60);
            lv_obj_set_size(buttonOff, 60, 50);
            applyCommonStyles(buttonOff);
            labelOff = lv_label_create(buttonOff);
            lv_label_set_text(labelOff, "OFF");
            lv_obj_center(labelOff);
            lv_obj_add_event_cb(buttonOff, buttonEventHandler, LV_EVENT_CLICKED, this);
            activityEventsLock(buttonOff, this);
        }

        // Flex container
        lv_obj_t* flexContainer = lv_obj_create(activeScreen);
        lv_obj_remove_style_all(flexContainer);
        lv_obj_set_size(flexContainer, canvasWidth - 60, 48);
        lv_obj_align(flexContainer, LV_ALIGN_TOP_MID, 0, 24);
        lv_obj_set_flex_flow(flexContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(flexContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(flexContainer, 8, 0);

        // Switch icon
        iconDeviceType = lv_label_create(flexContainer);
        lv_obj_set_style_text_font(iconDeviceType, &material_symbols_36, 0);
        lv_label_set_text(iconDeviceType, "\uE3A9");
        lv_obj_set_style_text_color(iconDeviceType, lv_color_white(), 0);

        // Module name
        if (labelTitle == nullptr) {
            static lv_anim_t animation_template;
            static lv_style_t label_style;
            lv_anim_init(&animation_template);
            lv_anim_set_delay(&animation_template, 1000);           //Wait 1 second to start the first scroll
            lv_anim_set_repeat_delay(&animation_template, 3000);    //Repeat the scroll 3 seconds after the label scrolls back to the initial position
            //Initialize the label style with the animation template
            lv_style_init(&label_style);
            lv_style_set_anim(&label_style, &animation_template);
            labelTitle = lv_label_create(flexContainer);
            lv_label_set_long_mode(labelTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);      //Circular scroll
            lv_obj_set_style_text_font(labelTitle, &lv_font_montserrat_24, 0);
            lv_label_set_text(labelTitle, module.name.c_str());
            lv_obj_add_style(labelTitle, &label_style, LV_STATE_DEFAULT);
            lv_obj_set_flex_grow(labelTitle, 1);
        }

        lv_obj_invalidate(activeScreen);
    }

    void LevelControlActivity::onPause() {
        if (activeScreen) {
            lv_obj_clean(activeScreen);
            activeScreen = nullptr;
        }
        sliderLevel = nullptr;
        labelLevel = nullptr;
        buttonOn = nullptr;
        labelOn = nullptr;
        buttonOff = nullptr;
        labelOff = nullptr;
        ledStatus = nullptr;
        labelTitle = nullptr;
        iconDeviceType = nullptr;

        lv_style_reset(&style_static_bg);

        LvglDriver::stop();
    }

    void LevelControlActivity::onDraw() {

        lv_timer_handler();
    }

}}}

#endif // DISABLE_LVGL
#endif // ENABLE_UI
