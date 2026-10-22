# HomeGenie ESP32Servo compatibility fixes

Vendored from the locally installed madhephaestus/ESP32Servo 3.2.1 sources. Original copyright and LGPL notices are retained in the sources. Used by smart-motor through its explicit local library dependency; no generated `.pio` files are patched.

Changes:
- Skip LEDC timer bookkeeping after MCPWM allocation (`timerNum` is -1).
- Route each S3 GPIO by its allocated MCPWM timer and A/B operator, independently of the unit number.
- Initialize a shared MCPWM timer once; attaching a second operator does not reset the first operator's duty. Reinitialize after the last allocation is released.
- Reuse vacant MCPWM operators without overwriting an active neighbour.
- Use a 14-bit default on S3 and initialize stored ticks consistently. The other targets retain their original defaults.

The smart-motor configuration, 50 Hz period, pulse bounds, initial level 50 and direction/positioning logic are unchanged.

Hardware acceptance reported by the maintainer on 2026-10-22:
- The combined robot-car and arm uses eight outputs (four wheels and four arm servos); operation was confirmed after the compatibility fixes.
- A remaining continuously turning wheel was caused by a stored angle maximum of 80 rather than 180. Correcting that configuration restored neutral operation. The historical angle-to-level mapping applies to continuous-rotation servos too.
- Website initialization presets now restore the intended wheel/arm ranges and the tested gripper settings; the maintainer confirmed initialization repairs a deliberately incorrect wheel setting. Firmware startup does not overwrite user angle limits.

Native tests cover eight independent S3 outputs and ESP32/C3 LEDC paths. They supplement hardware acceptance; they do not measure the physical PWM waveform. See `examples/smart-motor/README.md` for the tested gripper settings.
