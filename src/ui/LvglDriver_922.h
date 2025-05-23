//
// Created by gene on 15/05/25.
//

#ifndef HOMEGENIE_MINI_LVGLDRIVER_922_H
#define HOMEGENIE_MINI_LVGLDRIVER_922_H

#ifndef DISABLE_LVGL

#include <LovyanGFX.hpp>

#include "lv_conf.h"
#include <lvgl.h>


//Change to your screen resolution
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

//static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][ screenWidth * 10 ];

lv_display_t *display = lv_display_create(screenWidth, screenHeight);
/*Declare a buffer for 1/10 screen size*/
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
static uint8_t buf1[screenWidth * screenHeight / 10 * BYTE_PER_PIXEL];

LGFX_Device* lvgl_display = nullptr;
LGFX_Sprite* lvgl_gfx = nullptr;

/* Display flushing */
void my_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p)
{
    if (lvgl_gfx == nullptr) return;

    if (lvgl_gfx->getStartCount() == 0)
    {   // Processing if not yet started
        lvgl_gfx->startWrite();
    }
    lvgl_gfx->pushImageDMA( area->x1
            , area->y1
            , area->x2 - area->x1 + 1
            , area->y2 - area->y1 + 1
            , ( lgfx::swap565_t* )&color_p);
    lv_display_flush_ready(disp);
}


void sd_access_sample( void )
{
    if (lvgl_gfx == nullptr) return;

    if (lvgl_gfx->getStartCount() > 0)
    {   // Free the bus before accessing the SD card
        lvgl_gfx->endWrite();
    }

    // Something to manipulate the SD card.
//    auto file = SD.open("/file");
//    file.close();
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    if (lvgl_display == nullptr) return;

    uint16_t touchX, touchY;

    data->state = LV_INDEV_STATE_REL;

    if( lvgl_display->getTouch( &touchX, &touchY ) )
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
}


void lvgl_begin() {

    lv_init();

    lv_display_set_buffers(display, buf1, nullptr, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);  /*Initialize the display buffer.*/
    lv_display_set_flush_cb(display, my_disp_flush);

    lv_indev_t * indev = lv_indev_create();           /*Create an input device*/
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  /*Touch pad is a pointer-like device*/
    lv_indev_set_read_cb(indev, my_touchpad_read);    /*Set your driver function*/

}

#endif // DISABLE_LVGL

#endif //HOMEGENIE_MINI_LVGLDRIVER_922_H
