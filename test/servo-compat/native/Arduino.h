#pragma once
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <map>
#include <utility>
using boolean=bool;
using std::abs;
using std::pow;
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define ESP_ARDUINO_VERSION_MAJOR 3
#define ESP_ARDUINO_VERSION_VAL(a,b,c) ((a)*10000+(b)*100+(c))
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(3,3,12)
using note_t=int;
const int NOTE_MAX=12;
inline long map(long x,long in0,long in1,long out0,long out1){return (x-in0)*(out1-out0)/(in1-in0)+out0;}
