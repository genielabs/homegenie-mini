"""Run production Bluetooth completion and Config finalization with durable-state probes."""
from pathlib import Path
import os, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
if not (root/'src/Config.h').exists():root=Path(r'\\192.168.2.32\homegenie-mini')
def function(text,marker):
    start=text.index(marker);opening=text.index('{',start);depth=1;end=opening+1
    while depth:
        if text[end]=='{':depth+=1
        elif text[end]=='}':depth-=1
        end+=1
    return text[start:end]
header=(root/'src/Config.h').read_text(encoding='utf-8')
bluetooth=(root/'src/net/BluetoothManager.cpp').read_text(encoding='utf-8')
finish=function(header,'static bool onWiFiConfigured()')
getters='\n'.join(function(header,'static '+s) for s in [
    'bool isTemporaryConfiguration()', 'bool isConfigurationMode()',
    'bool hasWiFiCredentials()', 'bool isDeviceConfigured()','bool isWiFiConfigured()'])
handle=function(bluetooth,'void BluetoothManager::handleMessage(')
harness=r'''
#include <cassert>
#include <map>
#include <string>
#include <functional>
#include <cstdio>
#include <cstdint>
struct String:std::string {
    using std::string::string;using std::string::operator=;
    String(const std::string& s):std::string(s){}
    bool isEmpty()const{return empty();}bool equals(const char* s)const{return *this==s;}
};
const char *CONFIG_SYSTEM_NAME="fixture",*CONFIG_KEY_system_mode="mode",*CONFIG_KEY_config_once="once";
static std::map<std::string,String> strings;
static unsigned once=0,closed=0,restarts=0,callbacks=0;
static bool failOpen=false,failWrite=false,interruptTeardown=false;
struct Preferences {
    bool begin(const char*,bool){return !failOpen;}void end(){}
    void putString(const char* key,const char* value){if(!failWrite)strings[key]=value;}
    void putUInt(const char*,unsigned value){if(!failWrite)once=value;}
    String getString(const char* key,const char* fallback){return strings.count(key)?strings[key]:String(fallback);}
    unsigned getUInt(const char*,unsigned){return once;}
};
namespace DeviceSetup {enum class Mode{Normal,Temporary,Provisioning};}
struct Config {
    static DeviceSetup::Mode configurationMode;
    static bool legacyDisplayConfiguration;
    static struct System {String systemMode,ssid;} system;
    static std::function<void()> wifiConfiguredCallback;
    static void handleConfigCommand(String&);
'''
defs=r'''
};
DeviceSetup::Mode Config::configurationMode=DeviceSetup::Mode::Normal;
bool Config::legacyDisplayConfiguration=true;
Config::System Config::system;
std::function<void()> Config::wifiConfiguredCallback;
struct Restart{};
void Config::handleConfigCommand(String& message){if(message.equals("#RESET")){++restarts;throw Restart();}}
namespace IO {struct Logger{static void info(const char*,...){}static void error(const char*,...){}};}
struct Channel {
    void end(){
        // The exit must already be durable before a BLE teardown can interrupt.
        assert(strings["mode"].empty() && once==0);++closed;
        if(interruptTeardown)throw Restart();
    }
    void disconnect(){}
} channel;
class BluetoothManager {public:Channel* SerialBTLE=&channel;Channel SerialBT;void handleMessage(String&);};
'''
checks=r'''
int main(){
    String reset="#RESET";
    Config::wifiConfiguredCallback=[](){assert(strings["mode"].empty() && once==0);++callbacks;};
    for(auto mode:{DeviceSetup::Mode::Normal,DeviceSetup::Mode::Temporary,DeviceSetup::Mode::Provisioning}){
        for(bool persistent:{false,true}){
            strings={{"mode",persistent?"config":""},{"ssid","NEW"},{"password","new-secret"},{"$disp-typ","ST7796"}};
            once=2;Config::system.systemMode=persistent?"config":"";Config::system.ssid="";
            Config::configurationMode=mode;const auto oldClosed=closed,oldRestarts=restarts;
            BluetoothManager manager;
            try{manager.handleMessage(reset);assert(false);}catch(Restart&){}
            assert(closed>oldClosed && restarts==oldRestarts+1 && once==0 && strings["mode"].empty());
            // Emulate the next boot loading the newly stored credentials/mode.
            Config::configurationMode=DeviceSetup::Mode::Normal;
            Config::system.systemMode=strings["mode"];Config::system.ssid=strings["ssid"];
            assert(Config::isDeviceConfigured() && Config::isWiFiConfigured());
            assert(strings["password"]=="new-secret" && strings["$disp-typ"]=="ST7796");
        }
    }
    for(bool opening:{false,true}){
        strings["mode"]="config";once=2;Config::system.systemMode="config";
        failOpen=opening;failWrite=!opening;
        const auto oldClosed=closed,oldRestarts=restarts,oldCallbacks=callbacks;
        BluetoothManager manager;manager.handleMessage(reset);
        assert(closed==oldClosed && restarts==oldRestarts && callbacks==oldCallbacks);
        assert(strings["mode"]=="config" && once==2 && Config::system.systemMode=="config");
        failOpen=failWrite=false;
        try{manager.handleMessage(reset);assert(false);}catch(Restart&){}
        assert(strings["mode"].empty() && once==0);
    }
    strings["mode"]="config";once=2;interruptTeardown=true;BluetoothManager interrupted;
    try{interrupted.handleMessage(reset);assert(false);}catch(Restart&){}
    assert(strings["mode"].empty() && once==0);
    puts("PASS: production Bluetooth reset commits display Reconfigure exit before teardown; first-time/BOOT modes, restart, storage errors and retained display settings");
}
'''
with tempfile.TemporaryDirectory(prefix='mini-ble-completion-') as temp:
    source=Path(temp)/'test.cpp';exe=Path(temp)/('test.exe' if os.name=='nt' else 'test')
    source.write_text(harness+getters+'\n'+finish+defs+handle+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    for flags in ([],['-DDISABLE_BLUETOOTH_CLASSIC'],['-DDISABLE_BLUETOOTH_LE']):
        subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror']+flags+[str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
