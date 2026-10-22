#pragma once
#include "Arduino.h"
struct LedState {int channel,bits;double freq;unsigned duty=0;};
static std::map<int,LedState> led;
inline bool ledcAttachChannel(int pin,double freq,int bits,int ch){assert(ch>=0);if(!led.count(pin))led.emplace(pin,LedState{ch,bits,freq,0});return true;}
inline void ledcDetach(int pin){led.erase(pin);}
inline void ledcWrite(int pin,unsigned duty){assert(led.count(pin));led.at(pin).duty=duty;}
inline unsigned ledcRead(int pin){return led.at(pin).duty;}
