#include "PanelRelay.h"
#include "RelayKeyBody.h"
#ifdef ESP32
#include <Preferences.h>
#else
#include <LittleFS.h>
#endif
#include "HomeGenie.h"
#ifdef ESP32
#include <esp_http_client.h>
#include "PanelRelayRoots.h"
#include <atomic>
#else
#include "PanelRelay8266.h"
#endif

namespace PanelRelay {
static constexpr char RelayKey[] = "PYKqr2hUhdASrkWJGWuKpM+C7rbXaQTFcvFYz4d8IJBLK1bz4R6kw1";
namespace {
#ifdef ESP32
constexpr char KeyNamespace[] = "panel-relay";
constexpr char KeySetting[] = "custom";
SemaphoreHandle_t keyMutex = xSemaphoreCreateMutex();
struct KeyLock {
    bool held = keyMutex && xSemaphoreTake(keyMutex, portMAX_DELAY) == pdTRUE;
    ~KeyLock() { if (held) xSemaphoreGive(keyMutex); }
};
bool loadKey(String& key) {
    KeyLock lock;
    Preferences prefs;
    if (!lock.held || !prefs.begin(KeyNamespace, false)) return false;
    key = prefs.isKey(KeySetting) ? prefs.getString(KeySetting, "") : String(RelayKey);
    return !key.isEmpty();
}
bool storeKey(const char* key) {
    KeyLock lock;
    Preferences prefs;
    bool ok = lock.held && prefs.begin(KeyNamespace, false);
    if (ok) {
        if (!*key) ok = !prefs.isKey(KeySetting) || prefs.remove(KeySetting);
        else ok = prefs.getString(KeySetting, "") == key || prefs.putString(KeySetting, key) == strlen(key);
    }
    return ok;
}
#else
constexpr char CustomKeyPath[] = "/panel-relay.key";
constexpr char TempKeyPath[] = "/panel-relay.tmp";

bool loadKey(String& key) {
    if (!LittleFS.begin()) return false;
    if (!LittleFS.exists(CustomKeyPath)) {
        key = RelayKey;
        return !key.isEmpty();
    }
    auto file = LittleFS.open(CustomKeyPath, "r");
    if (!file || !file.size() || file.size() > KeyBody::MaxLength) return false;
    char buffer[KeyBody::MaxLength + 1];
    size_t size = file.size();
    if (file.read(reinterpret_cast<uint8_t*>(buffer), size) != size) return false;
    buffer[size] = 0;
    key = buffer;
    return key.length() == size;
}

bool storeKey(const char* key) {
    if (!LittleFS.begin()) return false;
    if (!*key) return !LittleFS.exists(CustomKeyPath) || LittleFS.remove(CustomKeyPath);
    auto file = LittleFS.open(TempKeyPath, "w");
    if (!file) return false;
    size_t size = strlen(key);
    bool written = file.write(reinterpret_cast<const uint8_t*>(key), size) == size;
    file.close();
    if (written && LittleFS.rename(TempKeyPath, CustomKeyPath)) return true;
    LittleFS.remove(TempKeyPath);
    return false;
}
#endif
}

const char* updateKey(const String& body) {
    char key[KeyBody::MaxLength + 1];
    if (!KeyBody::parse(body.c_str(), body.length(), key))
        return R"({"Status":"Error","Message":"Invalid relay key request"})";
    return storeKey(key) ? R"({"Status":"OK","Message":"Relay key updated"})"
              : R"({"Status":"Error","Message":"Cannot save relay key"})";
}

bool available() { return sizeof(RelayKey) > 1; }

#ifdef ESP32
static std::atomic<bool> sending{false};
bool busy() { return sending.load(); }
void loop() {}
static void worker(void* arg) {
    std::unique_ptr<String> payload(static_cast<String*>(arg));
    esp_http_client_config_t cfg{};
    cfg.url = "https://push.homegenie.workers.dev/";
    cfg.method = HTTP_METHOD_POST;
    cfg.cert_pem = RelayRoots;
    cfg.timeout_ms = 6000;
    cfg.disable_auto_redirect = true;
    auto client = esp_http_client_init(&cfg);
    int status = 0;
    String key;
    if (client && loadKey(key)) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_header(client, "x-hg-key", key.c_str());
        esp_http_client_set_post_field(client, payload->c_str(), payload->length());
        if (esp_http_client_perform(client) == ESP_OK) status = esp_http_client_get_status_code(client);
    }
    if (client) esp_http_client_cleanup(client);
    if (status < 200 || status >= 300) IO::Logger::warn("Panel relay delivery failed (%d)", status);
    payload.reset();
    sending.store(false);
    vTaskDelete(nullptr);
}
#endif

void send(const String& token, const String& message) {
    JsonDocument doc;
    doc["token"] = token;
    doc["title"] = "HomeGenie Update";
    doc["body"] = message;
    String body;
    if (doc.overflowed() || !body.reserve(measureJson(doc)) || !serializeJson(doc, body)) return;
#ifdef ESP32
    auto payload = new(std::nothrow) String(std::move(body));
    if (!payload) return;
    sending.store(true);
    if (xTaskCreate(worker, "PanelPush", 8192, payload, tskIDLE_PRIORITY + 1, nullptr) != pdPASS) {
        sending.store(false);
        delete payload;
        IO::Logger::warn("Panel relay worker unavailable");
    }
#else
    String key;
    if (loadKey(key)) startRequest(body, key.c_str());
    else IO::Logger::warn("Panel relay settings unavailable");
#endif
}
}
