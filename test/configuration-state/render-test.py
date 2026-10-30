"""Compile the actual renderPixels function with simple pixel/color test doubles."""
from pathlib import Path
import os, subprocess, sys, tempfile

root=Path(__file__).resolve().parents[2]
# Also runnable from the local preparation directory.
if not (root/'examples/color-light/color-light.h').exists():
    root=Path(__file__).resolve().parent/'repo'
text=(root/'examples/color-light/color-light.h').read_text(encoding='utf-8')
start=text.index('inline void renderPixels() {')
end=text.index('\ninline void createPixels()',start)
render=text[start:end]
harness=r'''
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
struct LightColor {
    float r=0,g=0,b=0;
    LightColor() {}
    LightColor(float a,float c,float d):r(a),g(c),b(d) {}
    float getRed() const {return r;}
    float getGreen() const {return g;}
    float getBlue() const {return b;}
    uint32_t getColor(float p=1) const {
        return uint32_t(round(r*p))<<16 | uint32_t(round(g*p))<<8 | uint32_t(round(b*p));
    }
};
struct {uint16_t count=256;} ledsConfig;
LightColor colors[256];
LightColor* animatedColors[256];
unsigned maxPower=25;
struct Pixels {
    uint32_t data[256]={};
    uint32_t getPixelColor(unsigned i) {assert(i<256);return data[i];}
    void setPixelColor(unsigned i,unsigned r,unsigned g,unsigned b) {
        assert(i<256);data[i]=r<<16 | g<<8 | b;
    }
} storage;
Pixels* pixels=&storage;
struct DirectLight {bool on=false;bool isOn(){return on;}} direct;
struct {
    int index=-1;
    bool exist(int i){return i==index;}
    DirectLight* get(int){return &direct;}
} colorLightList;
struct {
    LightColor current,effect;
    LightColor& getCurrentColor(){return current;}
    void setEffectColor(const LightColor& c){effect=c;}
} statusLed;
'''
checks=r'''
int main() {
    for(unsigned i=0;i<256;++i)animatedColors[i]=&colors[i];
    statusLed.current=LightColor(255,0,0);
    colors[17]=LightColor(0,0,255);
    renderPixels();
    assert(storage.data[0]==0 && storage.data[17]==64);
    assert(statusLed.effect.getColor()==0x0000ff);
    assert(colors[17].getColor()==0x0000ff);
    colors[17]=LightColor();
    renderPixels();
    assert(storage.data[17]==0 && statusLed.effect.getColor()==0xff0000);
    statusLed.current=LightColor();
    renderPixels();
    assert(statusLed.effect.getColor()==0);
    colors[255]=LightColor(0,255,0);
    renderPixels();
    assert(storage.data[255]==0x004000 && statusLed.effect.getColor()==0x00ff00);
    colorLightList.index=255;direct.on=true;storage.data[255]=0x123456;
    colors[255]=LightColor(255,0,0);
    renderPixels();
    assert(storage.data[255]==0x123456);
    // The status-only/no-strip path still displays active effect output.
    pixels=nullptr;ledsConfig.count=1;colors[0]=LightColor(3,7,9);
    renderPixels();
    assert(statusLed.effect.getColor()==0x030709);
    puts("PASS: real renderer, 16x16 sparse effects, current-color fallback, off, power limit, individual pixel override and status-only LED");
}
'''
with tempfile.TemporaryDirectory(prefix='mini-render-') as temp:
    folder=Path(temp);source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
    source.write_text(harness+render+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror',str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
