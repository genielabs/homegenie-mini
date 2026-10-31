"""Exercise production Config/WiFiManager methods with in-memory preferences."""
from pathlib import Path
import os, subprocess, sys, tempfile

root=Path(__file__).resolve().parents[2]
if not (root/'src/Config.h').exists():root=Path(__file__).resolve().parent/'repo'

def function(text,marker):
    start=text.index(marker);opening=text.index('{',start);depth=1;end=opening+1
    while depth:
        if text[end]=='{':depth+=1
        elif text[end]=='}':depth-=1
        end+=1
    return text[start:end]

header=(root/'src/Config.h').read_text(encoding='utf-8')
config=(root/'src/Config.cpp').read_text(encoding='utf-8')
wifi=(root/'src/net/WiFiManager.cpp').read_text(encoding='utf-8')
getters='\n'.join(function(header,'static '+signature) for signature in [
    'void enableLegacyDisplayConfiguration()', 'bool isTemporaryConfiguration()',
    'bool isConfigurationMode()', 'bool isFullConfiguration()', 'bool isBluetoothConfiguration()',
    'bool hasWiFiCredentials()', 'bool isDeviceConfigured()', 'bool isWiFiConfigured()'])
methods=function(config,'void Config::consumeConfigurationRequest()')+'\n'+function(config,'bool Config::restartInConfigurationMode(')
reset=function(wifi,'bool WiFiManager::configure(')
bluetooth=function((root/'src/net/BluetoothManager.cpp').read_text(encoding='utf-8'),'void BluetoothManager::begin()')
buttonMethod=function((root/'src/HomeGenie.h').read_text(encoding='utf-8'), 'static void checkServiceButton()')
harness=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include <string>
#define ESP32 1
#define DISABLE_BLUETOOTH_CLASSIC 1
#define CONFIG_BUILTIN_MODULE_NAME "mini"
#define CONFIG_SYSTEM_NAME "fixture"
const char* CONFIG_KEY_config_once="system:cfg1";
const char* CONFIG_KEY_wifi_ssid="wifi:ssid";
const char* CONFIG_KEY_wifi_password="wifi:password";
struct String:std::string {
    using std::string::string;using std::string::operator=;
    String(const std::string& text):std::string(text){}
    bool equals(const char* text)const{return *this==text;}
    bool isEmpty()const{return empty();}
};
struct Store {
    static bool failOpen,failWrite;
    static std::map<std::string,uint32_t> numbers;
    static std::map<std::string,String> strings;
};
bool Store::failOpen=false;bool Store::failWrite=false;
std::map<std::string,uint32_t> Store::numbers;
std::map<std::string,String> Store::strings;
struct Preferences {
    bool begin(const char*,bool){return !Store::failOpen;}
    void end(){}
    uint32_t getUInt(const char* key,uint32_t fallback){return Store::numbers.count(key)?Store::numbers[key]:fallback;}
    unsigned putUInt(const char* key,uint32_t value){if(Store::failWrite)return 0;Store::numbers[key]=value;return 4;}
    unsigned putString(const char* key,const char* value){if(Store::failWrite)return 0;Store::strings[key]=value;return Store::strings[key].size();}
    void clear(){Store::numbers.clear();Store::strings.clear();}
};
struct BleSerial {
    static unsigned starts;
    void begin(const char*){++starts;}
    void setTimeout(unsigned){}
};
unsigned BleSerial::starts=0;
struct BLEDevice {
    static bool released;
    static void init(const char*){}
    static void deinit(bool freeMemory){released=freeMemory;}
};
bool BLEDevice::released=false;
struct Restart{};
struct {unsigned calls=0;void restart(){++calls;throw Restart();}} ESP;
struct {void println(const char*){}} Serial;
void delay(unsigned){}
namespace IO {struct Logger{static void info(const char*,...){}};}
struct {bool erased=false;void disconnect(bool=false,bool erase=false){erased=erase;}} WiFi;
struct SystemConfig {String systemMode,ssid,pass;};
class Config {
public:
    static DeviceSetup::Mode configurationMode;
    static bool legacyDisplayConfiguration;
    static SystemConfig system;
    static short ServiceButtonPin;
    static bool wpsRequest,configurationRequested;
    static const unsigned ConfigureButtonPushInterval=3000;
    static void consumeConfigurationRequest();
    static bool restartInConfigurationMode(bool fullConfiguration=false);
'''
harness+=getters+'''
};
DeviceSetup::Mode Config::configurationMode=DeviceSetup::Mode::Normal;
bool Config::legacyDisplayConfiguration=false;
SystemConfig Config::system;
short Config::ServiceButtonPin=0;
bool Config::wpsRequest=false,Config::configurationRequested=false;
'''+methods+'\nnamespace Net {class WiFiManager{public:static bool configure(bool rebootIntoConfiguration=false);};\n'+reset+'''
class BluetoothManager {
public:
    BleSerial* SerialBTLE=nullptr;
    bool initialized=false;
    void begin();
    ~BluetoothManager(){delete SerialBTLE;}
};
'''+bluetooth+'\n}\n'
harness+=r'''
static unsigned long buttonTime=0;
static bool buttonPressed=false;
unsigned long millis(){return buttonTime;}
enum {LOW=0};
int digitalRead(short){return buttonPressed?LOW:1;}
struct ButtonLogger {static void error(const char*){}};
using Logger=ButtonLogger;
struct ButtonNetwork {
    Net::WiFiManager& getWiFiManager(){static Net::WiFiManager wifi;return wifi;}
};
class ButtonFixture {
public:
    DeviceSetup::ServiceButtonHold serviceButton;
    static ButtonFixture* getInstance(){static ButtonFixture fixture;return &fixture;}
    ButtonNetwork& getNetManager(){static ButtonNetwork network;return network;}
'''+buttonMethod+r'''
};
void assertBluetooth(bool);
void checkPhysicalSequence(){
    // Saved/empty credentials, ordinary device, first-time display and persistent display setup.
    for(bool saved:{false,true})for(unsigned displayMode=0;displayMode<3;++displayMode){
        Store::failOpen=false;Store::failWrite=false;Store::numbers.clear();Store::strings.clear();
        Config::configurationMode=DeviceSetup::Mode::Normal;
        Config::legacyDisplayConfiguration=displayMode!=0;
        Config::system.systemMode=displayMode==2?"config":"";
        Config::system.ssid=saved?"saved-network":"";
        Config::system.pass=saved?"saved-password":"";
        Store::strings[CONFIG_KEY_wifi_ssid]=Config::system.ssid;
        Store::strings[CONFIG_KEY_wifi_password]=Config::system.pass;
        Store::strings["$leds-cnt"]="256";Store::strings["script"]="keep-script";
        const auto original=Store::strings;
        ButtonFixture::getInstance()->serviceButton=DeviceSetup::ServiceButtonHold{};
        buttonTime=0;buttonPressed=false;ButtonFixture::checkServiceButton();
        const unsigned calls=ESP.calls;
        buttonPressed=true;buttonTime=100;ButtonFixture::checkServiceButton();
        buttonTime=131;ButtonFixture::checkServiceButton();
        buttonTime=4131;ButtonFixture::checkServiceButton();assert(ESP.calls==calls);
        buttonPressed=false;buttonTime=4200;ButtonFixture::checkServiceButton();
        buttonTime=4231;try{ButtonFixture::checkServiceButton();assert(false);}catch(const Restart&){}
        assert(ESP.calls==calls+1 && Store::strings==original);
        assert(Store::numbers[CONFIG_KEY_config_once]==uint32_t(DeviceSetup::Mode::Temporary));
        Config::consumeConfigurationRequest();
        assert(Config::isTemporaryConfiguration() && !Config::isFullConfiguration());
        assert(!Config::isBluetoothConfiguration());assertBluetooth(false);
        buttonPressed=true;buttonTime=4300;ButtonFixture::checkServiceButton();
        buttonTime=4331;ButtonFixture::checkServiceButton();
        buttonTime=8331;ButtonFixture::checkServiceButton();assert(ESP.calls==calls+1);
        buttonPressed=false;buttonTime=8400;ButtonFixture::checkServiceButton();
        buttonTime=8431;try{ButtonFixture::checkServiceButton();assert(false);}catch(const Restart&){}
        assert(Store::numbers[CONFIG_KEY_config_once]==uint32_t(DeviceSetup::Mode::Provisioning));
        assert(Store::strings[CONFIG_KEY_wifi_ssid].empty() && Store::strings[CONFIG_KEY_wifi_password].empty());
        assert(Store::strings["$leds-cnt"]=="256" && Store::strings["script"]=="keep-script");
        Config::system.ssid="";Config::system.pass="";Config::consumeConfigurationRequest();
        assert(Config::isFullConfiguration() && Config::isBluetoothConfiguration());assertBluetooth(true);
        Config::consumeConfigurationRequest();assert(!Config::isTemporaryConfiguration());
    }
}
'''
checks=r'''
void assertBluetooth(bool expected){
    const unsigned previous=BleSerial::starts;BLEDevice::released=false;
    Net::BluetoothManager manager;manager.begin();
    assert(manager.initialized==expected);
    assert(BleSerial::starts==previous+(expected?1:0));
    assert(BLEDevice::released==!expected);
}
int main(){
    // Ordinary operation without credentials and the display's explicit exception.
    assert(Config::isDeviceConfigured() && !Config::hasWiFiCredentials() && !Config::isWiFiConfigured());
    assert(!Config::isBluetoothConfiguration());assertBluetooth(false);
    Config::enableLegacyDisplayConfiguration();
    assert(Config::isConfigurationMode() && !Config::isDeviceConfigured() && Config::isBluetoothConfiguration());assertBluetooth(true);
    Config::system.ssid="saved-network";
    assert(Config::isDeviceConfigured() && Config::isWiFiConfigured());
    Config::system.systemMode="config";
    assert(Config::isConfigurationMode() && !Config::isTemporaryConfiguration() && Config::isBluetoothConfiguration());assertBluetooth(true);
    Config::system.systemMode="";Config::legacyDisplayConfiguration=false;
    Store::strings[CONFIG_KEY_wifi_ssid]="saved-network";
    Store::strings[CONFIG_KEY_wifi_password]="saved-password";
    Store::strings["$leds-cnt"]="256";
    Store::strings["$leds-rws"]="16";
    const auto original=Store::strings;
    try{Config::restartInConfigurationMode();assert(false);}catch(const Restart&){}
    assert(Store::strings==original && ESP.calls==1);
    Config::consumeConfigurationRequest();
    assert(Config::isTemporaryConfiguration() && !Config::isFullConfiguration() && !Config::isBluetoothConfiguration());assertBluetooth(false);
    // Explicit BOOT 1 also overrides the display's persistent BLE setup flag.
    Config::system.systemMode="config";assert(!Config::isBluetoothConfiguration());assertBluetooth(false);Config::system.systemMode="";
    assert(Config::hasWiFiCredentials() && !Config::isWiFiConfigured());
    assert(Store::numbers[CONFIG_KEY_config_once]==0);
    // A later ordinary reboot must return to normal and retain the network.
    Config::configurationMode=DeviceSetup::Mode::Normal;Config::consumeConfigurationRequest();
    assert(Config::isDeviceConfigured() && Config::isWiFiConfigured() && Store::strings==original);
    try{Config::restartInConfigurationMode();assert(false);}catch(const Restart&){}
    Config::consumeConfigurationRequest();
    try{Net::WiFiManager::configure(true);assert(false);}catch(const Restart&){}
    assert(WiFi.erased && Store::strings[CONFIG_KEY_wifi_ssid].empty() && Store::strings[CONFIG_KEY_wifi_password].empty());
    assert(Store::strings["$leds-cnt"]=="256" && Store::strings["$leds-rws"]=="16");
    Config::system.ssid="";Config::system.pass="";
    Config::configurationMode=DeviceSetup::Mode::Normal;Config::consumeConfigurationRequest();
    assert(Config::isTemporaryConfiguration() && Config::isFullConfiguration() && Config::isBluetoothConfiguration());assertBluetooth(true);
    Config::configurationMode=DeviceSetup::Mode::Normal;Config::consumeConfigurationRequest();
    assert(Config::isDeviceConfigured() && !Config::hasWiFiCredentials());
    // The first request without credentials is WPS-only and does not erase settings.
    WiFi.erased=false;
    const auto offline=Store::strings;
    try{Config::restartInConfigurationMode();assert(false);}catch(const Restart&){}
    Config::consumeConfigurationRequest();
    assert(Config::isTemporaryConfiguration() && !Config::isFullConfiguration() && !Config::isBluetoothConfiguration());assertBluetooth(false);
    assert(!WiFi.erased && Store::strings==offline);
    // No sticky configuration on the next ordinary boot.
    Config::configurationMode=DeviceSetup::Mode::Normal;Config::consumeConfigurationRequest();
    assert(Config::isDeviceConfigured() && !Config::isBluetoothConfiguration());assertBluetooth(false);
    // Consuming BOOT 1 without credentials must keep WPS-only mode.
    Store::numbers[CONFIG_KEY_config_once]=1;Config::consumeConfigurationRequest();
    assert(Config::isTemporaryConfiguration() && !Config::isFullConfiguration() && !Config::isBluetoothConfiguration());assertBluetooth(false);
    unsigned calls=ESP.calls;Store::failWrite=true;
    assert(!Config::restartInConfigurationMode() && ESP.calls==calls);
    Store::failWrite=false;Store::failOpen=true;
    assert(!Config::restartInConfigurationMode() && !Net::WiFiManager::configure(true) && ESP.calls==calls);
    checkPhysicalSequence();
    puts("PASS: physical WPS-first/Bluetooth-second gestures, saved/empty credentials and display modes; production config entry/reset, one-shot reboot, credential preservation, 16x16 settings, WPS-only/BLE selection, offline entry, display legacy modes and storage failures");
}
'''
with tempfile.TemporaryDirectory(prefix='mini-config-core-') as temp:
    folder=Path(temp);source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
    include='#include "'+(root/'src/ConfigurationState.h').as_posix()+'"\n'
    source.write_text(include+harness+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror',str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
