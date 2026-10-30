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
    'bool WiFiManager::saveTemporaryWpsCredentials()', 'void WiFiManager::temporaryWpsLoop()',
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
    bool isConnected(){return connected;}
    void disconnect(bool=false,bool erase=false){connected=false;if(erase)sdkErased=storageMode==2;}
    bool beginWPSConfig(){return true;}
    void setAutoReconnect(bool){} void persistent(bool){}
} WiFi;
struct System {String ssid="OLD",pass="old-secret";};
struct Config {
    static System system;static int notified;
    static bool hasWiFiCredentials(){return !system.ssid.isEmpty();}
    static void onWiFiConfigured(){++notified;}
    static bool restartInConfigurationMode(bool){throw Reboot();}
    static void statusLedOn(){} static void statusLedOff(){}
};
System Config::system;int Config::notified=0;
struct {int restarts=0;void restart(){++restarts;throw Reboot();}void eraseConfig(){sdkErased=true;}} ESP;
void delay(unsigned){} void yield(){}
namespace IO {struct Logger {static void info(const char*,...){} static void error(const char*,...){} };}
static bool startWorks=true;static int starts=0, connects=0, stops=0;
enum { WIFI_STORAGE_RAM=1, WIFI_STORAGE_FLASH=2, ESP_OK=0, WPS_TYPE_PBC=0, WIFI_IF_STA=0 };
struct wifi_config_t {struct {unsigned char ssid[32]; unsigned char password[64];} sta;};
int esp_wifi_get_config(int,wifi_config_t* config){
    *config={};std::memcpy(config->sta.ssid,WiFi.ssid.c_str(),WiFi.ssid.size());
    std::memcpy(config->sta.password,WiFi.password.c_str(),WiFi.password.size());return ESP_OK;
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
    enum class WpsPhase {Initial,Registering,Associating,Restart,Finished};
    WpsPhase temporaryWpsPhase=WpsPhase::Initial;
    unsigned long temporaryWpsTimestamp=0;
    bool temporaryWpsConnectRequested=false;
    static void temporaryWpsCallback(int){}
    static bool wpsStart(){++starts;return startWorks;}
    static void wpsStop(){++stops;}
    bool startTemporaryWps();void stopTemporaryWps();
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
    storageMode=2;sdkErased=false;openFails=writeFails=false;startWorks=true;starts=connects=stops=ESP.restarts=0;
    WiFiManager::temporaryWpsEvents.store(0);
}
void paired(WiFiManager& w){
    WiFiManager::temporaryWpsEvents.fetch_or(1);w.temporaryWpsLoop();
    assert(w.temporaryWpsPhase==WiFiManager::WpsPhase::Associating);
    now+=1999;w.temporaryWpsLoop();assert(connects==0);
    now+=1;w.temporaryWpsLoop();assert(connects==1);
}
void ip(WiFiManager& w){WiFi.ssid="NEW";WiFi.password="new-secret";WiFi.connected=true;
    WiFiManager::temporaryWpsEvents.fetch_or(4);w.temporaryWpsLoop();}
int main(){
    reset();WiFiManager w;w.temporaryWpsLoop();assert(starts==1);
    // An old AP's GOT_IP before WPS success must not save or reboot.
    WiFi.connected=true;WiFiManager::temporaryWpsEvents.fetch_or(4);w.temporaryWpsLoop();
    assert(Config::notified==0 && saved["ssid"]=="OLD");WiFi.connected=false;
    paired(w);ip(w);assert(saved["ssid"]=="NEW" && saved["pass"]=="new-secret" && Config::notified==1);
    now+=999;w.temporaryWpsLoop();assert(ESP.restarts==0);
    now+=1;try{w.temporaryWpsLoop();assert(false);}catch(Reboot&){}assert(ESP.restarts==1);
    reset();WiFiManager timeout;timeout.temporaryWpsLoop();now+=40000;timeout.temporaryWpsLoop();
    assert(timeout.temporaryWpsPhase==WiFiManager::WpsPhase::Finished && saved["ssid"]=="OLD" && Config::notified==0);
    reset();WiFiManager failed;failed.temporaryWpsLoop();WiFiManager::temporaryWpsEvents.fetch_or(2);failed.temporaryWpsLoop();
    assert(failed.temporaryWpsPhase==WiFiManager::WpsPhase::Finished && saved["pass"]=="old-secret");
    reset();Config::system.pass="";saved["pass"]="";WiFiManager legacy;legacy.temporaryWpsLoop();
    assert(saved["pass"]=="old-secret" && Config::system.pass=="old-secret");
    // Simulated power loss here: canonical OLD credentials survive SDK mutation.
    WiFi.ssid="NEW";WiFi.password="wrong-network";assert(saved["ssid"]=="OLD" && saved["pass"]=="old-secret");
    reset();WiFiManager noIp;noIp.temporaryWpsLoop();paired(noIp);now+=15000;noIp.temporaryWpsLoop();
    assert(noIp.temporaryWpsPhase==WiFiManager::WpsPhase::Finished && saved["ssid"]=="OLD");
    reset();Config::system.ssid="";Config::system.pass="";saved["ssid"]="";saved["pass"]="";
    WiFiManager erased;erased.temporaryWpsLoop();now+=40000;erased.temporaryWpsLoop();
    assert(saved["ssid"].isEmpty() && saved["pass"].isEmpty());
    reset();WiFiManager storage;storage.temporaryWpsLoop();paired(storage);writeFails=true;ip(storage);
    assert(storage.temporaryWpsPhase==WiFiManager::WpsPhase::Finished && saved["ssid"]=="OLD" && Config::notified==0);
    reset();Config::system.pass="";openFails=true;WiFiManager backup;backup.temporaryWpsLoop();
    assert(starts==0 && backup.temporaryWpsPhase==WiFiManager::WpsPhase::Finished);
    reset();startWorks=false;WiFiManager start;start.temporaryWpsLoop();
    assert(start.temporaryWpsPhase==WiFiManager::WpsPhase::Finished && saved["ssid"]=="OLD");
    // Unsigned clock rollover must not delay restart or timeout indefinitely.
    reset();now=0xffffff00UL;WiFiManager wrap;wrap.temporaryWpsLoop();now+=40000;wrap.temporaryWpsLoop();
    assert(wrap.temporaryWpsPhase==WiFiManager::WpsPhase::Finished);
    reset();WiFiManager boot2;boot2.temporaryWpsLoop();
    // Even serial #WPS during temporary setup must erase the persistent SDK copy.
    try{WiFiManager::configure();assert(false);}catch(Reboot&){}
    assert(sdkErased && saved["ssid"].isEmpty() && saved["pass"].isEmpty());
    puts("PASS: WPS success/restart, old-IP rejection, failure/timeouts, previous-credential migration, reset mode and storage failures");
}
'''
with tempfile.TemporaryDirectory(prefix='mini-wps-') as temp:
    folder=Path(temp); source=folder/'test.cpp';exe=folder/('test.exe' if os.name=='nt' else 'test')
    source.write_text(harness+methods+checks,encoding='utf-8')
    compiler=sys.argv[1:] or ['c++']
    for platform in ('ESP32','ESP8266'):
        subprocess.run(compiler+['-std=c++11','-Wall','-Wextra','-Werror','-D'+platform,'-DCONFIGURE_WITH_WPS',str(source),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
