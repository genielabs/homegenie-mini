# Auxiliary controller LED regressions

Run with Python and a C++11 compiler:

```sh
python test/auxiliary-led/test.py c++
```

The test compiles the production AuxiliaryLed and StatusLed classes with minimal
hardware/API doubles, both with and without BOARD_HAS_RGB_LED. It checks disabled
allocation, C3/S3 controller pins, C1/C2 separation, status/effect replication,
overlay/activity restoration, mode persistence, independent output frames,
dirty refresh and rejection of shared LED-output pins. Hardware/API firmware
acceptance is still required; doubles do not replace NeoPixel transport or the
standard ColorLight transition implementation.
