"""Exercise production LED resize, renderer and script-range code with allocation failures."""
from pathlib import Path
import os, subprocess, sys, tempfile
root = Path(__file__).resolve().parents[2]
if not (root/'examples/color-light/color-light.h').exists():
    root = Path(r'\\192.168.2.32\homegenie-mini')
def function(text, marker):
    start=text.index(marker); opening=text.index('{',start); depth=1; end=opening+1
    while depth:
        if text[end]=='{': depth+=1
        elif text[end]=='}': depth-=1
        end+=1
    return text[start:end]
header=(root/'examples/color-light/color-light.h').read_text()
fx=(root/'examples/color-light/fx/fx.h').read_text()
script=(root/'examples/color-light/fx/fx_script.h').read_text()
count_branch=header.split('} else if (option->is(Options::Controller::LED_count)) {',1)[1].split('} else if',1)[0]
assert 'disposePixels' not in count_branch and 'ledsConfig.count =' not in count_branch
assert 'applyPendingLedCount();' in (root/'examples/color-light/color-light.cpp').read_text()
harness=r'''
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
static int live=0,failColor=-1,updates=0;
struct LightColor {
    int tag=0; LightColor(){++live;} LightColor(const LightColor& c):tag(c.tag){++live;} ~LightColor(){--live;}
    static void* operator new(size_t n,const std::nothrow_t&) noexcept {
        if(failColor==0)return nullptr; if(failColor>0)--failColor; return std::malloc(n);
    }
    static void operator delete(void* p) noexcept {std::free(p);}
    void setColor(float,float,float,unsigned){++updates;}
    float getHue()const{return .5f;} float getSaturation()const{return 1;} float getValue()const{return 1;}
    float getTargetHue()const{return .5f;} float getTargetSaturation()const{return 1;} float getTargetValue()const{return 1;}
    int getRed()const{return 1;} int getGreen()const{return 2;} int getBlue()const{return 3;}
};
typedef LightColor* AnimatedColor;
AnimatedColor* animatedColors=nullptr; uint16_t animatedColorCount=0;
struct MatrixConfig {uint16_t count=1,cols=1,rows=1;} ledsConfig;
struct String:std::string {
    using std::string::string;String(int n):std::string(std::to_string(n)){}
    bool equals(const char* s)const{return *this==s;}
};
String operator+(const String& a,const String& b){return String((std::string(a)+b).c_str());}
namespace IO {namespace IOEventDomains {const char* HomeAutomation_HomeGenie="domain";}
struct Logger {static void error(const char*,...){} };}
namespace Configuration {const char* LedsModules="leds-mod";}
namespace Options {namespace Controller {const char* LED_count="LED.count";}}
bool perPixel=false;
struct Config {static String getSetting(const char*,const char*){return perPixel?"true":"false";}};
struct ColorLight {
    std::function<void(LightColor)> callback;
    ColorLight(const char*,const char*,const char*){}
    void onSetColor(std::function<void(LightColor)> c){callback=c;}
};
template<class T> struct LinkedList:std::vector<T> {
    int size()const{return static_cast<int>(std::vector<T>::size());}
    void add(T t){this->push_back(t);}
};
LinkedList<ColorLight*> colorLightList;
std::atomic<uint16_t> requestedLedCount{0};
struct Option {unsigned saved=1;void setValue(const char* s){saved=std::atoi(s);requestedLedCount.store(saved);}} option;
struct Module {Option* getProperty(const char*){return &option;}} module;
struct HomeGenie {int handlers=0;Module* getDefaultModule(){return &module;}void addAPIHandler(ColorLight*){++handlers;}} hg;
HomeGenie* homeGenie=&hg;
static bool badPixels=false;static int pixelLive=0,begins=0,shows=0;
struct Adafruit_NeoPixel {
    uint16_t count;
    Adafruit_NeoPixel(uint16_t n,int,int):count(badPixels?0:n){++pixelLive;}
    ~Adafruit_NeoPixel(){--pixelLive;}
    uint16_t numPixels(){return count;}
    void setPixelColor(unsigned i,int,int,int){assert(i<count);}
    void begin(){++begins;}
};
Adafruit_NeoPixel* pixels=nullptr;
int16_t ledsPin=8,pixelsType=82,pixelsSpeed=0;
uint8_t maxPower=25;
struct {LightColor color;LightColor& getCurrentColor(){return color;}} statusLed;
void refresh(){++shows;}
unsigned long millis(){return 100;}
struct {unsigned long startTime=0;} player;
decltype(player)* scriptHandler=&player;
bool styleChanged=false,levelChanged=false;
enum {CMD_ON,CMD_OFF,CMD_DEFAULT,CMD_TOGGLE,CMD_LEVEL,CMD_HSB};
'''
methods='\n'.join([
    function(fx,'inline bool fx_resize('),
    function(header,'inline void setupPixelModules()'),
    function(header,'inline void createPixels()'),
    function(header,'inline void disposePixels()'),
    function(header,'inline void applyPendingLedCount()'),
    function(script,'inline void executeOnLedRange(')])
checks=r'''
void resize(unsigned n){requestedLedCount.store(n);applyPendingLedCount();}
int main(){
    (void)scriptHandler;
    const int baseline=live;
    assert(fx_resize(1,statusLed.color));createPixels();auto first=animatedColors[0];first->tag=42;
    resize(256);assert(ledsConfig.count==256 && animatedColorCount==256 && pixels->numPixels()==256);
    assert(animatedColors[0]==first && first->tag==42 && pixelLive==1 && live==baseline+256);
    assert(styleChanged && levelChanged && player.startTime==100);
    resize(16);assert(live==baseline+16 && animatedColors[0]==first);
    // Previously this explicit range swapped its clipped end back out of bounds.
    updates=0;executeOnLedRange(200,255,CMD_ON,statusLed.color,0,0,0,0,16);assert(updates==0);
    executeOnLedRange(255,200,CMD_ON,statusLed.color,0,0,0,0,16);assert(updates==0);
    executeOnLedRange(255,4,CMD_ON,statusLed.color,0,0,0,0,16);assert(updates==12);
    updates=0;executeOnLedRange(0,0xffff,CMD_ON,statusLed.color,0,0,0,0,16);assert(updates==16);
    // Repeated increases/decreases neither leak colors nor duplicate handlers.
    perPixel=true;setupPixelModules();assert(hg.handlers==16);
    for(int i=0;i<30;++i){resize(256);assert(live==baseline+256);resize(16);assert(live==baseline+16);}
    assert(hg.handlers==256 && colorLightList.size()==256);
    colorLightList[255]->callback(statusLed.color); // inactive tail must not write
    colorLightList[0]->callback(statusLed.color);
    // Fail midway through growth: all old objects, hardware and count survive.
    auto oldPixels=pixels;failColor=3;resize(128);
    assert(pixels==oldPixels && ledsConfig.count==16 && animatedColorCount==16 && live==baseline+16);
    assert(option.saved==16 && animatedColors[0]==first && pixelLive==1);
    applyPendingLedCount();failColor=-1;resize(128);assert(animatedColorCount==128);
    // Simulate Adafruit's internal malloc failing without throwing.
    oldPixels=pixels;badPixels=true;resize(64);
    assert(pixels==oldPixels && animatedColorCount==128 && ledsConfig.count==128 && pixelLive==1);
    applyPendingLedCount();badPixels=false;resize(64);assert(animatedColorCount==64);
    // No LED strip: status-only devices can still resize their effect buffer.
    disposePixels();ledsPin=-1;resize(7);assert(pixels==nullptr && animatedColorCount==7);
    assert(fx_resize(0,statusLed.color));assert(live==baseline && pixelLive==0);
    for(auto light:colorLightList)delete light;
    puts("PASS: production resize 1/256/16, repeated growth/shrink, allocation rollback, stable handlers, script clipping and status-only output");
}
'''
with tempfile.TemporaryDirectory(prefix='mini-resize-') as temp:
    source=Path(temp)/'test.cpp';exe=Path(temp)/('test.exe' if os.name=='nt' else 'test')
    source.write_text(harness+methods+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    for flags in ([],['-DDISABLE_AUTOMATION']):
        # The no-automation case does not reset the absent script player.
        text=harness+methods+checks
        if flags:text=text.replace(' && player.startTime==100','')
        source.write_text(text,encoding='utf-8')
        subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror']+flags+[str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
