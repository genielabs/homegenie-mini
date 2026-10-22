# ESP32Servo host regressions

Compile the real vendored ESP32Servo implementation against minimal hardware mocks. From the repository root, using a C++14 compiler:

```sh
c++ -std=c++14 -DARDUINO -DCONFIG_IDF_TARGET_ESP32S3 -Itest/servo-compat/native -Ilib/ESP32Servo/src test/servo-compat/native/test.cpp -o /tmp/servo-s3-test
/tmp/servo-s3-test
```

Repeat without `-DCONFIG_IDF_TARGET_ESP32S3` for classic ESP32 LEDC; use `-DCONFIG_IDF_TARGET_ESP32C3 -DARDUINO_ESP32C3_DEV` for the C3 LEDC path. The mocks model Arduino core 3.x.

S3 checks eight unique outputs across both MCPWM units, neutral pulse accuracy, independent commands, preservation of an active neighbour during attach and detach/reattach, and timer reuse. These are host regressions, not an embedded build or a measurement of the PWM waveform.
