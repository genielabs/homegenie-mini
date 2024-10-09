// SSDPDevice.h
// based on https://github.com/esp8266/Arduino/issues/2283 
// https://github.com/esp8266/Arduino/files/980894/SSDPDevice.zip
// by Pawel Dino

#ifndef _SSDPDEVICE_h
#define _SSDPDEVICE_h

#include "lwip/igmp.h"

#ifdef ESP8266 
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>

#define pip41(ipaddr) ((u16_t)(((u8_t*)(ipaddr))[0]))
#define pip42(ipaddr) ((u16_t)(((u8_t*)(ipaddr))[1]))
#define pip43(ipaddr) ((u16_t)(((u8_t*)(ipaddr))[2]))
#define pip44(ipaddr) ((u16_t)(((u8_t*)(ipaddr))[3]))


#define LIP2STR(ipaddr) pip41(ipaddr), \
    pip42(ipaddr), \
    pip43(ipaddr), \
    pip44(ipaddr)


#define SSDP_INTERVAL     1200
#define SSDP_PORT         1900
//#define SSDP_METHOD_SIZE  10
//#define SSDP_URI_SIZE     2
//#define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 2

#define SSDP_QUEUE_SIZE 21

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);

#define SSDP_UUID_SIZE              37
#define SSDP_SCHEMA_URL_SIZE        64
#define SSDP_DEVICE_TYPE_SIZE       64
#define SSDP_FRIENDLY_NAME_SIZE     64
#define SSDP_SERIAL_NUMBER_SIZE     32
#define SSDP_PRESENTATION_URL_SIZE  128
#define SSDP_MODEL_NAME_SIZE        64
#define SSDP_MODEL_DESCRIPTION_SIZE        64
#define SSDP_MODEL_URL_SIZE         128
#define SSDP_MODEL_VERSION_SIZE     32
#define SSDP_MANUFACTURER_SIZE      64
#define SSDP_MANUFACTURER_URL_SIZE  128

typedef enum {
	NOTIFY_ALIVE_INIT,
	NOTIFY_ALIVE,
	NOTIFY_UPDATE,
	RESPONSE
} ssdp_message_t;

typedef enum {
	ROOT_FOR_ALL,
	ROOT_BY_UUID,
	ROOT_BY_TYPE
} ssdp_udn_t;

typedef struct {
	unsigned long time;
	
	ssdp_message_t type;
	ssdp_udn_t udn;
	uint32_t address;
	uint16_t port;
} ssdp_send_parameters_t;

class SSDPDeviceClass {
private:
	WiFiUDP *m_server;

	IPAddress m_last;

	char m_schemaURL[SSDP_SCHEMA_URL_SIZE];
	char m_uuid[SSDP_UUID_SIZE];
	char m_deviceType[SSDP_DEVICE_TYPE_SIZE];
	char m_friendlyName[SSDP_FRIENDLY_NAME_SIZE];
	char m_serialNumber[SSDP_SERIAL_NUMBER_SIZE];
	char m_presentationURL[SSDP_PRESENTATION_URL_SIZE];
	char m_manufacturer[SSDP_MANUFACTURER_SIZE];
	char m_manufacturerURL[SSDP_MANUFACTURER_URL_SIZE];
	char m_modelName[SSDP_MODEL_NAME_SIZE];
    char m_modelDescription[SSDP_MODEL_DESCRIPTION_SIZE];
	char m_modelURL[SSDP_MODEL_URL_SIZE];
	char m_modelNumber[SSDP_MODEL_VERSION_SIZE];
	
	uint16_t m_port;
	uint8_t m_ttl;

	ssdp_send_parameters_t m_queue[SSDP_QUEUE_SIZE];
protected:
	bool readLine(String &value);
	bool readKeyValue(String &key, String &value);

	void postNotifyALive();
	void postNotifyUpdate();
	void postResponse(long mx);
	void postResponse(ssdp_udn_t udn, long mx);
	void post(ssdp_message_t type, ssdp_udn_t udn, IPAddress address, uint16_t port, unsigned long time);
	
	void send(ssdp_send_parameters_t *parameters);
public:
	SSDPDeviceClass();

	void update();

	void schema(WiFiClient client);

	void handleClient();

    String getId();

	void setDeviceType(const String& deviceType) { setDeviceType(deviceType.c_str()); }
	void setDeviceType(const char *deviceType);
	void setName(const String& name) { setName(name.c_str()); }
	void setName(const char *name);
	void setURL(const String& url) { setURL(url.c_str()); }
	void setURL(const char *url);
	void setSchemaURL(const String& url) { setSchemaURL(url.c_str()); }
	void setSchemaURL(const char *url);
	void setSerialNumber(const String& serialNumber) { setSerialNumber(serialNumber.c_str()); }
	void setSerialNumber(const char *serialNumber);
	void setSerialNumber(const uint32_t serialNumber);
	void setModelName(const String& name) { setModelName(name.c_str()); }
	void setModelName(const char *name);
    void setModelDescription(const String& name) { setModelDescription(name.c_str()); }
    void setModelDescription(const char *name);
	void setModelNumber(const String& num) { setModelNumber(num.c_str()); }
	void setModelNumber(const char *num);
	void setModelURL(const String& url) { setModelURL(url.c_str()); }
	void setModelURL(const char *url);
	void setManufacturer(const String& name) { setManufacturer(name.c_str()); }
	void setManufacturer(const char *name);
	void setManufacturerURL(const String& url) { setManufacturerURL(url.c_str()); }
	void setManufacturerURL(const char *url);
    void setFriendlyName(const String& url) { setFriendlyName(url.c_str()); }
    void setFriendlyName(const char *url);
	void setHTTPPort(uint16_t port);
	void setTTL(uint8_t ttl);
};

extern SSDPDeviceClass SSDPDevice;

#endif
