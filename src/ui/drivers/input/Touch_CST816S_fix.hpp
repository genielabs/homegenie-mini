/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
 [genemars](https://github.com/genemars)
/----------------------------------------------------------------------------*/

#ifndef HOMEGENIE_MINI_TOUCH_CST816S_FIX_H
#define HOMEGENIE_MINI_TOUCH_CST816S_FIX_H

#ifdef ENABLE_UI

#include <LovyanGFX.h>
#include <lgfx/v1/Touch.hpp>

namespace UI { namespace Drivers { namespace Input {

  using namespace lgfx;

  struct Touch_CST816S_fix : public ITouch
  {
    enum CS816S_GESTURE {
      NONE         = 0x00,
      SWIPE_UP     = 0x01,
      SWIPE_DOWN   = 0x02,
      SWIPE_LEFT   = 0x03,
      SWIPE_RIGHT  = 0x04,
      SINGLE_CLICK = 0x05,
      DOUBLE_CLICK = 0x0B,
      LONG_PRESS   = 0x0C
    };

    Touch_CST816S_fix(void)
    {
      _cfg.i2c_addr = 0x15;
      _cfg.x_min = 0;
      _cfg.x_max = 320;
      _cfg.y_min = 0;
      _cfg.y_max = 320;
    }

    bool init(void) override;

    void wakeup(void) override {};
    void sleep(void) override {};

    uint_fast8_t getTouchRaw(touch_point_t* tp, uint_fast8_t count) override;

  private:
    enum
    {
      max_touch_points = 1
    };
    uint32_t _last_update = 0;
    uint8_t _wait_cycle = 0;

    bool _check_init(void);
    bool _write_reg(uint8_t reg, uint8_t val);
    bool _write_regs(uint8_t* val, size_t length);
    bool _read_reg(uint8_t reg, uint8_t *data, size_t length);
    size_t _read_data(uint8_t* data);
  };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_TOUCH_CST816S_FIX_H