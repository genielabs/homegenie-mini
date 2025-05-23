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

#ifndef HOMEGENIE_MINI_ACTIVITY_H
#define HOMEGENIE_MINI_ACTIVITY_H

#include "defs.h"

#ifdef ENABLE_UI

#include <LovyanGFX.hpp>
#include <LinkedList.h>

#include "Task.h"
#include "InputControl.h"
#include "GestureHelper.h"

#define CONFIG_KEY_ACTIVITY_TITLE "title-"

namespace UI {

    class Activity: private Task, public PointerHandler, public PointerListener, public GestureListener {

    public:

        virtual void attach(LGFX_Device* displayDevice) {
            display = displayDevice;
        }

        void setDrawInterval(uint64_t ms) {
            Task::setLoopInterval(ms);
        }

        /**
         * Sets the task in a "paused" state. All loop/refresh activities
         * should be stopped here and release resources when possible.
         */
        void pause() {
            if (!isPaused) {
                isPaused = true;
                onPause();
                canvas->deleteSprite();
            }
        }
        /**
         * Resumes the task from a paused state. The task may re-allocate
         * resources and start loops/refresh activities.
         */
        void resume() {
            if (isPaused) {
                isPaused = false;
                initCanvas();
//                canvas->clear();
                float h_center = (float)display->width() / 2.0f;
                float v_center = (float)display->height() / 2.0f;
                display->setPivot(h_center, v_center);
                onResume();
            }
        }
        /**
         * Event fired when the task is resumed.
         */
        virtual void onResume() {}
        /**
         * Event fired when the task is paused.
         */
        virtual void onPause() {}
        /**
         * Event fired to draw/refresh activity UI content.
         */
        virtual void onDraw() {}

        void setDrawOffset(float x, float y) {
            bool forceRedraw = false;
            if (drawOffset.x == 0 || x == 0) {
                forceRedraw = true;
            }
            drawOffset.x = x;
            drawOffset.y = y;
            canvas->setPivot(((float)canvas->width() / 2.0f) - x, canvas->getPivotY());
// todo:            canvas->setClipRect(...)
            if (forceRedraw) {
                onDraw();
            }
            canvas->pushRotateZoom(0, 1, 1);
        }


        // pointer listener
        void pointerDown(float x, float y) override {
            auto e = PointerEvent(x, y);
            uint8_t i = 0;
            for (const auto &item: inputControls) {
                if (item->hitTest(x, y)) {
                    item->isPressed = true;
                    item->draw();
                    e.target = item;
                    item->onPointerDown(e);
                    break;
                }
                i++;
            }
            onPointerDown(e);
        }
        void pointerUp(float x, float y) override {
            auto e = PointerEvent(x, y);
            uint8_t i = 0;
            for (const auto &item: inputControls) {
                if (item->isPressed || item->hitTest(x, y)) {
                    item->isPressed = false;
                    item->draw();
                    e.target = item;
                    item->onPointerUp(e);
                }
                i++;
            }
            onPointerUp(e);
        }
        void pointerMove(float x, float y) override {
            auto e = PointerEvent(x, y);
            uint8_t i = 0;
            for (const auto &item: inputControls) {
                if (item->isPressed && item->hitTest(x, y)) {
                    e.target = item;
                    item->onPointerMove(e);
                } else if (item->isPressed) {
                    item->isPressed = false;
                    item->draw();
                }
                i++;
            }
            onPointerMove(e);
        }
        // pointer listener
        void onPointerDown(PointerEvent e) override {}
        void onPointerUp(PointerEvent e) override {}
        void onPointerMove(PointerEvent e) override {}

        // gesture handler
        void tap(PointerEvent e) {
            uint8_t i = 0;
            for (const auto &item: inputControls) {
                if (item->hitTest(e.x, e.y)) {
                    e.target = item;
                    item->onTap(PointerEvent{e.x, e.y,item});
                    break;
                }
                i++;
            }
            onTap(e);
        }
        // gesture listener
        void onTap(PointerEvent e) override {}
        void onSwipe(SwipeEvent e) override {}
        void pan(PanEvent e) {
            uint8_t i = 0;
            for (const auto &item: inputControls) {
                if (item->hitTest(e.touchPoint.x, e.touchPoint.y)) {
                    e.target = item;
                    item->onPan(e);
                    break;
                }
                i++;
            }
            onPan(e);
        }
        void onPan(PanEvent e) override {}
        void onTouch(PointerEvent e) override {}
        void onRelease(PointerEvent e) override {}

        void addControl(InputControl* control) {
            inputControls.add(control);
            control->draw();
        }

        void lock() {
            locked = true;
        }
        void unlock() {
            locked = false;
        }
        bool isLocked() const {
            return locked;
        }

    protected:
        LGFX_Device* display = nullptr;
        LGFX_Sprite* canvas = nullptr;
        struct Offset {
        public:
            float x;
            float y;
        } drawOffset;

        void invalidate() {
            draw();
        }

        void initCanvas() {
            if (canvas == nullptr) {
                canvas = new LGFX_Sprite(display);
#ifdef BOARD_HAS_PSRAM
                canvas->setPsram(true);
#endif
                canvas->initDMA();
            }
            canvas->setColorDepth(lgfx::color_depth_t::grayscale_1bit);
            canvas->createSprite(display->width(), display->height());
        }

    private:
        LinkedList<InputControl*> inputControls;
        bool isPaused = true;
        bool locked = false;

        bool willLoop() override {
            bool canLoop = !isPaused && this->Task::willLoop();
            if (canLoop) {
                resume();
            }
            return canLoop;
        }
        void loop() override {
            draw();
        }
        void draw() {
            onDraw();
            for (const auto &item: inputControls) {
                item->draw();
            }
            canvas->pushRotateZoom(0, 1, 1);
        }

    };

}

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_ACTIVITY_H
