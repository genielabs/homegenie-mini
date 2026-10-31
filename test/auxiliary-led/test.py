"""Exercise the production AuxiliaryLed/StatusLed classes with hardware/API doubles.
Usage: python test.py <C++ compiler> [compiler prefix arguments]
"""
from pathlib import Path
import os, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
def header(name):
    return '\n'.join(line for line in (root/'examples/color-light'/name).read_text(encoding='utf-8').splitlines()
                     if not line.startswith('#include') and line!='#pragma once')
shim=r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <vector>
#include <functional>
struct String:std::string {
    using std::string::string;using std::string::operator=;
    String(const std::string& s):std::string(s){}
    bool equals(const char* s)const{return *this==s;}
    bool isEmpty()const{return empty();}
    int toInt()const{return std::atoi(c_str());}
};
static unsigned now=0;
struct {bool connected=false;bool isConnected(){return connected;}} WiFi;
unsigned millis(){return now;}
struct Task {virtual void loop()=0;void setLoopInterval(unsigned){}};
struct Config {
    static std::map<std::string,String> settings;
    static bool isStatusLedOn;static bool statusLedConfigurationActive;static short StatusLedPin;
    static bool isDeviceConfigured(){return true;}
    static std::function<void(bool)> callback;
    static String getSetting(const char* key,const char* fallback=""){
        return settings.count(key)?settings[key]:String(fallback);
    }
    static void statusLedCallback(std::function<void(bool)> f){callback=f;}
};
std::map<std::string,String> Config::settings;
bool Config::statusLedConfigurationActive=false;
bool Config::isStatusLedOn=false;short Config::StatusLedPin=-1;
std::function<void(bool)> Config::callback;
namespace IO {
namespace IOEventDomains {static const char* HomeAutomation_HomeGenie="HomeAutomation.HomeGenie";}
struct Logger {static unsigned errors;static void error(const char*){++errors;}};
unsigned Logger::errors=0;
}
namespace Data {
struct ModuleParameter {
 struct UpdateListener{virtual void onUpdate(ModuleParameter*)=0;};
 String name,value,key;UpdateListener* listener=nullptr;
 ModuleParameter(const char* n,const char* v):name(n),value(v){}
 bool is(const char* n){return name==n;}
 ModuleParameter* withConfigKey(const char* k){key=k;value=Config::getSetting(k,value.c_str());return this;}
 ModuleParameter* addUpdateListener(UpdateListener* l){listener=l;return this;}
 void setValue(const char* v){value=v;Config::settings[key]=v;if(listener)listener->onUpdate(this);}
};
struct Module {
 std::map<std::string,String> properties;ModuleParameter* mode=nullptr;
 void setProperty(const char* k,const char* v){properties[k]=v;}
 ModuleParameter* addWidgetOption(const char* n,const char* v,const char* definition){
  assert(std::string(definition)=="select:Auxiliary LED mode:status|independent");
  mode=new ModuleParameter(n,v);return mode;
 }
};
}
#define UI_WIDGETS_FIELD_TYPE_SELECT "select"
namespace Service {namespace API {
namespace WidgetApi {namespace Implements {
static const char* Scheduling="Scheduling";static const char* Scheduling_ModuleEvents="ModuleEvents";
}}
namespace devices {
struct LightColor {
 float h=0,s=0,v=0;
 void setColor(float a,float b,float c,unsigned){h=a;s=b;v=c;}
 float getHue()const{return h;}float getSaturation()const{return s;}float getValue()const{return v;}
 uint32_t getColor()const{return uint32_t(h*255)<<16|uint32_t(s*255)<<8|uint32_t(v*255);}
};
struct ColorLight {
 Data::Module* module=new Data::Module();String address;
 std::function<void(LightColor)> callback;
 ColorLight(const char*,const char* a,const char*):address(a){}
 void onSetColor(std::function<void(LightColor)> f){callback=f;}
 void emit(uint32_t color){LightColor c;c.setColor(((color>>16)&255)/255.f,((color>>8)&255)/255.f,(color&255)/255.f,0);callback(c);}
};
}}
struct HomeGenie {
 std::vector<API::devices::ColorLight*> handlers;
 static HomeGenie* getInstance(){static HomeGenie h;return &h;}
 void addAPIHandler(API::devices::ColorLight* c){handlers.push_back(c);}
};
}
struct Adafruit_NeoPixel {
 static std::vector<Adafruit_NeoPixel*> drivers;
 unsigned pin,type,shows=0;uint32_t color=0,displayed=0;
 Adafruit_NeoPixel(unsigned count,int p,int t):pin(p),type(t){assert(count==1);drivers.push_back(this);}
 void begin(){}
 void setPixelColor(unsigned i,uint32_t c){assert(i==0);color=c;}
 void setPixelColor(unsigned i,unsigned r,unsigned g,unsigned b){setPixelColor(i,Color(r,g,b));}
 uint32_t getPixelColor(unsigned i){assert(i==0);return color;}
 void show(){++shows;displayed=color;}
 static uint32_t Color(unsigned r,unsigned g,unsigned b){return r<<16|g<<8|b;}
};
std::vector<Adafruit_NeoPixel*> Adafruit_NeoPixel::drivers;
"""
checks=r"""
void reset(){WiFi.connected=false;Config::statusLedConfigurationActive=false;Config::settings.clear();Config::callback=nullptr;Config::isStatusLedOn=false;Config::StatusLedPin=-1;
 Adafruit_NeoPixel::drivers.clear();Service::HomeGenie::getInstance()->handlers.clear();IO::Logger::errors=0;now=0;}
LightColor red(){LightColor c;c.setColor(1,0,0,0);return c;}
void indicate(bool value){assert(Config::callback);Config::isStatusLedOn=value;Config::callback(value);}
int main(){
 reset();{StatusLed status;status.setup();assert(Adafruit_NeoPixel::drivers.empty());assert(Service::HomeGenie::getInstance()->handlers.size()==1);assert(!Config::callback);}
 for(const char* gpio:{"8","9"}){
  reset();Config::settings["auld-pin"]=gpio;Config::settings["leds-pin"]="5";
#ifdef BOARD_HAS_RGB_LED
  Config::settings["stld-pin"]="10";
#endif
  StatusLed status;status.setup();
  auto& handlers=Service::HomeGenie::getInstance()->handlers;
  assert(handlers.size()==2&&handlers[0]->address=="C1"&&handlers[1]->address=="C2");
  auto aux=Adafruit_NeoPixel::drivers.back();assert(aux->pin==(unsigned)std::atoi(gpio)&&aux->type==82);
  auto c2=handlers[1];assert(c2->module->mode->value=="status");
  status.setEffectColor(red());status.show();assert(aux->displayed==0xff0000);
  unsigned shows=aux->shows;status.show();status.setEffectColor(red());status.show();assert(aux->shows==shows);
  c2->emit(0x0000ff);assert(aux->displayed==0xff0000);
  indicate(true);assert(aux->displayed==0x646400);
  status.setEffectColor(red());status.show();assert(aux->displayed==0x646400);
  indicate(false);assert(aux->displayed==0xff0000);
  status.signalActivity(0,255,0);assert(aux->displayed==0x00ff00);
  status.setEffectColor(red());status.show();assert(aux->displayed==0x00ff00);
  now=51;status.loop();assert(aux->displayed==0xff0000);
  c2->module->mode->setValue("independent");assert(Config::settings["auld-mod"]=="independent");assert(aux->displayed==0x0000ff);
  status.setEffectColor(red());status.show();indicate(true);indicate(false);status.signalActivity(255,255,255);now+=51;status.loop();assert(aux->displayed==0x0000ff);
  c2->emit(0);assert(aux->displayed==0);c2->emit(0x000080);assert(aux->displayed==0x000080);
  c2->module->mode->setValue("status");assert(aux->displayed==0xff0000);
  c2->module->mode->setValue("independent");assert(aux->displayed==0x000080);
 }
 reset();Config::settings["auld-pin"]="8";Config::settings["auld-mod"]="independent";
 {StatusLed status;status.setup();auto aux=Adafruit_NeoPixel::drivers.back();assert(aux->displayed==0);status.setEffectColor(red());status.show();indicate(true);assert(aux->displayed==0);}
 // Historical connected window and ongoing loss/setup overlays, including aux-only boards.
 reset();Config::settings["auld-pin"]="8";WiFi.connected=true;
 {StatusLed status;status.setup();auto aux=Adafruit_NeoPixel::drivers.back();
  status.setEffectColor(red());status.show();indicate(true);assert(aux->displayed==0x646400);
  now=20001;status.loop();assert(aux->displayed==0xff0000);
  indicate(false);indicate(true);assert(aux->displayed==0xff0000);
  // The ignored core pulse must not freeze the normal effect.
  LightColor green;green.setColor(0,1,0,0);status.setEffectColor(green);status.show();assert(aux->displayed==0x00ff00);
  WiFi.connected=false;indicate(false);indicate(true);assert(aux->displayed==0x646400);
  WiFi.connected=true;indicate(false);indicate(true);assert(aux->displayed==0x00ff00);
  Config::statusLedConfigurationActive=true;indicate(true);assert(aux->displayed==0x646400);
  Config::statusLedConfigurationActive=false;indicate(true);assert(aux->displayed==0x00ff00);
 }
 // All three existing LED outputs must be rejected before allocating a C2/driver.
 for(unsigned conflict=0;conflict<3;++conflict){
  reset();Config::settings["auld-pin"]="8";
  if(conflict==0)Config::settings["leds-pin"]="8";
  if(conflict==1)Config::StatusLedPin=8;
  AuxiliaryLed aux;assert(!aux.setup(conflict==2?8:-1));assert(IO::Logger::errors==1);assert(Adafruit_NeoPixel::drivers.empty());assert(Service::HomeGenie::getInstance()->handlers.empty());
 }
 puts("PASS: production aux/status classes; disabled footprint, C3/S3 pins, C1/C2 separation, replica, BOOT/loss overlay, activity restore, live/persistent modes, independent colors/off/dim frames, dirty refresh, pin conflicts and aux-only boards");
}
"""
with tempfile.TemporaryDirectory(prefix='mini-aux-') as tmp:
 folder=Path(tmp);source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
 source.write_text(shim+header('AuxiliaryLed.h')+header('StatusLed.h')+checks,encoding='utf-8')
 for defines in [[],['-DBOARD_HAS_RGB_LED']]:
  subprocess.run((sys.argv[1:] or ['c++'])+['-std=c++11','-O0']+defines+[str(source),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True)
