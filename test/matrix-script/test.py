"""Compile the real matrix script compiler with a minimal Arduino String adapter.
Usage: python test.py <C++ compiler> [compiler prefix args, e.g. zig.exe c++]
"""
from pathlib import Path
import os, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
config=(root/'examples/color-light/configuration.h').read_text(encoding='utf-8')
compiler=(root/'examples/color-light/fx/fx_script_utils.h').read_text(encoding='utf-8').replace('#include "../configuration.h"','')
shim=r"""
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cassert>
class String {
    std::string s;
public:
    String(const char* v=""):s(v){} String(std::string v):s(v){}
    int length() const{return (int)s.size();}
    char operator[](unsigned i)const{return s[i];}
    const char* c_str()const{return s.c_str();}
    bool startsWith(const char* v)const{return s.find(v)==0;}
    int indexOf(char c,int start=0)const{auto i=s.find(c,start);return i==s.npos?-1:(int)i;}
    int lastIndexOf(char c)const{auto i=s.rfind(c);return i==s.npos?-1:(int)i;}
    String substring(unsigned a,unsigned b=UINT32_MAX)const{
        if(a>b) std::swap(a,b); if(a>=s.size())return String();
        return String(s.substr(a,std::min<size_t>(b,s.size())-a));
    }
    void trim(){auto a=s.find_first_not_of(" \r\n\t");auto b=s.find_last_not_of(" \r\n\t");s=a==s.npos?"":s.substr(a,b-a+1);}
    void toUpperCase(){for(char& c:s)c=(char)std::toupper((unsigned char)c);}
    void replace(const char* a,const char* b){size_t i=0;while((i=s.find(a,i))!=s.npos){s.replace(i,strlen(a),b);i+=strlen(b);}}
    long toInt()const{return std::strtol(s.c_str(),nullptr,10);}
    float toFloat()const{return std::strtof(s.c_str(),nullptr);}
    bool operator==(const char* v)const{return s==v;}
};
inline void yield(){}
"""
checks=r"""
static uint16_t u16(const std::vector<uint8_t>& b,size_t i){return b.at(i)|(b.at(i+1)<<8);}
struct Result {std::vector<int> hits,values;int commands;};
Result execute(const String& s,const MatrixConfig& cfg){
    std::vector<uint8_t> b;assert(compileScript(s,b,cfg));
    Result result{std::vector<int>(cfg.count),std::vector<int>(cfg.count),0};
    size_t pos=0;
    while(pos<b.size()){
        assert(pos+6<=b.size());unsigned count=u16(b,pos+4);pos+=6;
        for(unsigned i=0;i<count;++i){
            unsigned op=b.at(pos++);
            if(op==CMD_JUMP||op==CMD_JUMP_REPEAT){
                unsigned extra=op==CMD_JUMP_REPEAT?1:0;
                unsigned target=u16(b,pos+1+extra);assert(target<b.size());
                pos+=7+extra;continue;
            }
            unsigned a=u16(b,pos),z=u16(b,pos+2);pos+=4;
            if(z==65535){a=0;z=cfg.count-1;}
            if(a>z)std::swap(a,z);assert(z<cfg.count);
            int value=op==CMD_OFF?0:1;
            if(op==CMD_DEFAULT||op==CMD_LEVEL){value=b.at(pos);pos+=3;}
            if(op==CMD_HSB){value=1000+b.at(pos)*65536+b.at(pos+1)*256+b.at(pos+2);pos+=5;}
            for(unsigned j=a;j<=z;++j){++result.hits[j];result.values[j]=value;}
            ++result.commands;
        }
    }
    assert(pos==b.size());return result;
}
int main(){
    unsigned tests=0;
    for(unsigned bits=0;bits<16;++bits){
        for(auto dims:std::vector<std::pair<unsigned,unsigned>>{{16,16},{9,5}}){
            MatrixConfig cfg{(uint16_t)(dims.first*dims.second),(uint16_t)dims.first,(uint16_t)dims.second,
                bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8)};
            for(const char* selection:{"1:2+4:3","1:2-3:5","3:5-1:2","1:2+4"}){
                bool row=std::string(selection)=="1:2+4";
                auto result=execute(String(std::string("00.000 ")+selection+" TOGGLE"),cfg);
                std::vector<int> expected(cfg.count);
                for(unsigned y=1;y<=(row?1u:3u);++y)for(unsigned x=2;x<=5;++x)expected[getLEDIndex(x,y,cfg)]=1;
                assert(result.hits==expected);
                assert(result.commands==(cfg.isVertical?4:(row?1:3)));
                ++tests;
            }
            for(const char* selection:{"0-7","0+8"}){
                auto result=execute(String(std::string("00.000 ")+selection+" ON"),cfg);
                for(unsigned i=0;i<cfg.count;++i)assert(result.hits[i]==(i<8?1:0));
                assert(result.commands==1);++tests;
            }
            auto all=execute("00.000 ALL OFF",cfg);for(int n:all.hits)assert(n==1);++tests;
            auto point=execute("00.000 2:3 ON",cfg);assert(point.hits[getLEDIndex(3,2,cfg)]==1);++tests;
            auto clipped=execute("00.000 3:7+65535:65535 ON",cfg);
            std::vector<int> expected(cfg.count);
            for(unsigned y=3;y<cfg.rows;++y)for(unsigned x=7;x<cfg.cols;++x)expected[getLEDIndex(x,y,cfg)]=1;
            assert(clipped.hits==expected);++tests;
        }
    }
    const char* ghost=GHOST_SCRIPT;
    MatrixConfig base{256,16,16,false,true,false,false};auto original=execute(ghost,base);
    for(unsigned bits=0;bits<16;++bits){
        MatrixConfig cfg{256,16,16,bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8)};
        auto actual=execute(ghost,cfg);
        for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x)
            assert(actual.values[getLEDIndex(x,y,cfg)]==original.values[getLEDIndex(x,y,base)]);
        ++tests;
    }
    printf("PASS: %u cases; real compiler/bytecode, 16 layouts, rectangular matrices, spans, rectangles, reversed endpoints, clipping, physical ranges, ALL, ghost colors and loop targets\n",tests);
}
"""
# The site ghost is kept as a fixture: no dependency on the website/repo at test time.
ghost='00.000 ALL Off\n#1\n00.000 1:4+8 DEFAULT\n00.000 2:2+12 DEFAULT\n00.000 3:1+14:10 DEFAULT\n00.000 13:1+3 DEFAULT\n00.000 13:5+6 DEFAULT\n00.000 13:12+3 DEFAULT\n00.000 14:1 DEFAULT\n00.000 14:5+2 DEFAULT\n00.000 14:9+2 DEFAULT\n00.000 14:14 DEFAULT\n00.000 4:4+4:4 HSB(0,0,1,0)\n00.000 4:10+4:4 HSB(0,0,1,0)\n00.000 6:5+2:2 Off\n00.000 6:11+2:2 Off\n#LOOP:1'
import json
checks=checks.replace('GHOST_SCRIPT',json.dumps(ghost))
with tempfile.TemporaryDirectory(prefix='mini-script-') as tmp:
    folder=Path(tmp);source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
    source.write_text(shim+config+compiler+checks,encoding='utf-8')
    subprocess.run((sys.argv[1:] or ['c++'])+['-std=c++11','-O0',str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
