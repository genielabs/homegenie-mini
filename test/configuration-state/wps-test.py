"""Run the production temporary-WPS state machine with deterministic SDK events.
Usage: python3 test/configuration-state/wps-test.py c++
"""
from pathlib import Path
import os, subprocess, sys, tempfile
root = Path(__file__).resolve().parents[2]
def function(text, marker):
    start=text.index(marker); opening=text.index('{',start); depth=1; end=opening+1
    while depth:
        if text[end]=='{': depth+=1
        elif text[end]=='}': depth-=1
        end+=1
    return text[start:end]
cpp=(root/'src/net/WiFiManager.cpp').read_text()
methods='\n'.join(function(cpp,'    '+signature) for signature in [
    'bool WiFiManager::startTemporaryWps()', 'void WiFiManager::stopTemporaryWps()',
    'bool WiFiManager::saveTemporaryWpsCredentials()', 'void WiFiManager::retryTemporaryWps(const char* reason)', 'void WiFiManager::temporaryWpsLoop()',
    'bool WiFiManager::configure(bool rebootIntoConfiguration)'])
harness=r'''
#include <atomic>
#include <map>
#include <string>
#include <cassert>
#include <cstdio>
#include <cstring>
struct String:std::string {
    using std::string::string;using std::string::operator=;
    String(const std::string& s):std::string(s){}
    bool isEmpty()const{return empty();}
};
struct Reboot{};
static int storageMode=0;static bool sdkErased=false;
static unsigned long now=100;
static int arduinoBegins=0;static bool dhcpReady=false;
unsigned long millis(){return now;}
const char *CONFIG_SYSTEM_NAME="fixture", *CONFIG_KEY_wifi_ssid="ssid", *CONFIG_KEY_wifi_password="pass";
static std::map<std::string,String> saved;
static bool openFails=false, writeFails=false;
struct Preferences {
    bool begin(const char*,bool){return !openFails;}
    void end(){}
    void putString(const char* key,const String& value){if(!writeFails)saved[key]=value;}
    String getString(const char* key,const char* fallback){return saved.count(key)?saved[key]:String(fallback);}
};
struct {String ssid="OLD",password="old-secret";bool connected=false;
    String SSID(){
#ifdef ESP32
        if (!connected) return ""; // Current core returns only the associated AP.
#endif
        return ssid;
    }
    String psk(){return password;}
    int begin(){++arduinoBegins;dhcpReady=true;return 6;}
    bool isConnected(){return connected;}
    void disconnect(bool=false,bool erase=false){connected=false;if(erase)sdkErased=storageMode==2;}
    bool beginWPSConfig(){return true;}
    void setAutoReconnect(bool){} void persistent(bool){}
} WiFi;
struct System {String ssid="OLD",pass="old-secret";};
struct Config {
    static System system;static int notified;
    static bool hasWiFiCredentials(){return !system.ssid.isEmpty();}
    static bool onWiFiConfigured(){++notified;return true;}
    static bool restartInConfigurationMode(bool){throw Reboot();}
    static void statusLedOn(){} static void statusLedOff(){}
};
System Config::system;int Config::notified=0;
struct {int restarts=0;void restart(){++restarts;throw Reboot();}void eraseConfig(){sdkErased=true;}} ESP;
void delay(unsigned){} void yield(){}
namespace IO {struct Logger {static void info(const char*,...){} static void error(const char*,...){} };}
static bool configReadable=true;
static bool startWorks=true;static int starts=0, connects=0, stops=0;
enum { WIFI_STORAGE_RAM=1, WIFI_STORAGE_FLASH=2, ESP_OK=0, WPS_TYPE_PBC=0, WIFI_IF_STA=0 };
struct wifi_config_t {struct {unsigned char ssid[32]; unsigned char password[64];} sta;};
int esp_wifi_get_config(int,wifi_config_t* config){
    if(!configReadable)return -1;
    *config={};std::memcpy(config->sta.ssid,WiFi.ssid.c_str(),WiFi.ssid.size());
    std::memcpy(config->sta.password,WiFi.password.c_str(),WiFi.password.size());return ESP_OK;
}
struct station_config {unsigned char ssid[32]; unsigned char password[64];};
bool wifi_station_get_config(station_config* config){
    if(!configReadable)return false;
    *config={};std::memcpy(config->ssid,WiFi.ssid.c_str(),WiFi.ssid.size());
    std::memcpy(config->password,WiFi.password.c_str(),WiFi.password.size());return true;
}
int esp_wifi_set_storage(int mode){storageMode=mode;return ESP_OK;}
int esp_wifi_connect(){++connects;return 0;}
void disable_extra4k_at_link_time(){}
bool wifi_wps_disable(){++stops;return true;}
bool wifi_wps_enable(int){return true;}
bool wifi_set_wps_cb(void(*)(int)){return true;}
bool wifi_wps_start(){++starts;return startWorks;}
void wifi_station_connect(){++connects;}
namespace Net {
class WiFiManager {
public:
    static std::atomic<unsigned> temporaryWpsEvents;
    static bool temporaryWps,esp32_wps_started;
    static bool configure(bool rebootIntoConfiguration=false);
    enum class WpsPhase {Initial,Registering,Restart,Retry};
    WpsPhase temporaryWpsPhase=WpsPhase::Initial;
    unsigned long temporaryWpsTimestamp=0;
    static void temporaryWpsCallback(int){}
    static bool wpsStart(){++starts;return startWorks;}
    static void wpsStop(){++stops;}
    bool startTemporaryWps();void stopTemporaryWps();void retryTemporaryWps(const char* reason);
    bool saveTemporaryWpsCredentials();void temporaryWpsLoop();
};
std::atomic<unsigned> WiFiManager::temporaryWpsEvents{0};
bool WiFiManager::temporaryWps=true; bool WiFiManager::esp32_wps_started=false;
'''
checks=r'''
}
using Net::WiFiManager;
void reset(){now=100;saved={{"ssid","OLD"},{"pass","old-secret"}};
    Config::system.ssid="OLD";Config::system.pass="old-secret";Config::notified=0;
    WiFi.ssid="OLD";WiFi.password="old-secret";WiFi.connected=false;
    storageMode=2;sdkErased=false;openFails=writeFails=false;startWorks=configReadable=true;
    starts=connects=stops=ESP.restarts=0;
    WiFiManager::temporaryWpsEvents.store(0);arduinoBegins=0;dhcpReady=false;
}
void paired(WiFiManager& w){
    WiFi.ssid="NEW";WiFi.password="new-secret";
    WiFiManager::temporaryWpsEvents.fetch_or(1);w.temporaryWpsLoop();
}
void reboot(WiFiManager& w){
    assert(w.temporaryWpsPhase==WiFiManager::WpsPhase::Restart);
    assert(!WiFi.connected && arduinoBegins==0 && connects==0);
    now+=999;w.temporaryWpsLoop();assert(ESP.restarts==0);
    now+=1;try{w.temporaryWpsLoop();assert(false);}catch(Reboot&){}
    assert(ESP.restarts==1);
}
void retry(WiFiManager& w){
    assert(w.temporaryWpsPhase==WiFiManager::WpsPhase::Retry && ESP.restarts==0);
    now+=1999;w.temporaryWpsLoop();assert(w.temporaryWpsPhase==WiFiManager::WpsPhase::Retry);
    WiFiManager::temporaryWpsEvents.fetch_or(1u|4u);
    now+=1;w.temporaryWpsLoop();assert(w.temporaryWpsPhase==WiFiManager::WpsPhase::Registering);
    assert(WiFiManager::temporaryWpsEvents.exchange(0)==0);
}
int main(){
    reset();WiFiManager w;w.temporaryWpsLoop();assert(starts==1);
    // IP alone, even from the old network, must not save or reboot.
    WiFi.connected=true;WiFiManager::temporaryWpsEvents.fetch_or(4);w.temporaryWpsLoop();
    assert(Config::notified==0 && saved["ssid"]=="OLD");WiFi.connected=false;
    paired(w);assert(saved["ssid"]=="NEW" && saved["pass"]=="new-secret" && Config::notified==1);
    reboot(w);
    // No old credentials and no IP: successful WPS is sufficient.
    reset();Config::system.ssid="";Config::system.pass="";saved["ssid"]="";saved["pass"]="";
    WiFiManager firstTime;firstTime.temporaryWpsLoop();paired(firstTime);reboot(firstTime);
    assert(saved["ssid"]=="NEW" && saved["pass"]=="new-secret");
    reset();WiFiManager repeated;repeated.temporaryWpsLoop();
    for(int i=0;i<4;++i){now+=40000;repeated.temporaryWpsLoop();retry(repeated);}
    assert(starts==5 && saved["ssid"]=="OLD" && saved["pass"]=="old-secret");
    WiFiManager::temporaryWpsEvents.fetch_or(2u);repeated.temporaryWpsLoop();retry(repeated);
    writeFails=true;paired(repeated);
    assert(saved["ssid"]=="OLD" && saved["pass"]=="old-secret" && Config::notified==0);
    writeFails=false;retry(repeated);paired(repeated);reboot(repeated);
    reset();WiFiManager unreadable;unreadable.temporaryWpsLoop();configReadable=false;paired(unreadable);
    assert(saved["ssid"]=="OLD" && Config::notified==0);
    configReadable=true;retry(unreadable);paired(unreadable);reboot(unreadable);
    // Reject missing credentials even when the SDK reports WPS success.
    reset();WiFiManager empty;empty.temporaryWpsLoop();WiFi.ssid="";WiFi.password="";
    WiFiManager::temporaryWpsEvents.fetch_or(1);empty.temporaryWpsLoop();
    assert(empty.temporaryWpsPhase==WiFiManager::WpsPhase::Retry && saved["ssid"]=="OLD");
    // Maximum-length fields need no NUL in the SDK's fixed-size buffers.
    reset();WiFiManager full;full.temporaryWpsLoop();WiFi.ssid=std::string(32,'s');WiFi.password=std::string(64,'p');
    WiFiManager::temporaryWpsEvents.fetch_or(1);full.temporaryWpsLoop();
    assert(saved["ssid"].size()==32 && saved["pass"].size()==64);reboot(full);
    // Preserve legacy passwords before WPS can replace SDK credentials.
    reset();Config::system.pass="";saved["pass"]="";WiFiManager legacy;legacy.temporaryWpsLoop();
    assert(saved["pass"]=="old-secret" && Config::system.pass=="old-secret");
    reset();startWorks=false;WiFiManager cannotStart;cannotStart.temporaryWpsLoop();
    now+=2000;cannotStart.temporaryWpsLoop();assert(cannotStart.temporaryWpsPhase==WiFiManager::WpsPhase::Retry);
    startWorks=true;retry(cannotStart);assert(starts==3);
    reset();Config::system.pass="";saved["pass"]="";openFails=true;
    WiFiManager unavailableStorage;unavailableStorage.temporaryWpsLoop();
    openFails=false;retry(unavailableStorage);assert(saved["pass"]=="old-secret");
    reset();now=0xffffff00UL;WiFiManager wrap;wrap.temporaryWpsLoop();now+=40000;wrap.temporaryWpsLoop();
    assert(wrap.temporaryWpsPhase==WiFiManager::WpsPhase::Retry);retry(wrap);
    reset();WiFiManager boot2;boot2.temporaryWpsLoop();
    try{WiFiManager::configure();assert(false);}catch(Reboot&){}
    assert(sdkErased && saved["ssid"].isEmpty() && saved["pass"].isEmpty());
    puts("PASS: save SDK credentials and restart on WPS success without IP; retries, storage/read failures, full-length fields, old-IP rejection and credential reset");
}

'''
with tempfile.TemporaryDirectory(prefix='mini-wps-') as temp:
    folder=Path(temp); source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
    source.write_text(harness+methods+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    for platform in ('ESP32','ESP8266'):
        subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror','-D'+platform,'-DCONFIGURE_WITH_WPS',str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
