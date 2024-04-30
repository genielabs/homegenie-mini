#pragma once
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLE2902.h>
#include "ByteRingBuffer.h"


#define BLE_BUFFER_SIZE ESP_GATT_MAX_ATTR_LEN //must be greater than MTU, less than ESP_GATT_MAX_ATTR_LEN
#define MIN_MTU 50
#define RX_BUFFER_SIZE 4096

class BleSerial : public BLECharacteristicCallbacks, public BLEServerCallbacks, public Stream
{
public:
	BleSerial() = default;
    BleSerial(BleSerial const &other) = delete;		 // disable copy constructor
    void operator=(BleSerial const &other) = delete; // disable assign constructor

	void begin(const char *name, bool enable_led = false, int led_pin = 13);
	static void end();
	void onWrite(BLECharacteristic *pCharacteristic) override;
	int available() override;
	int read() override;
	size_t readBytes(uint8_t *buffer, size_t bufferSize) override;
	int peek() override;
	size_t write(uint8_t byte) override;
	void flush() override;
	size_t write(const uint8_t *buffer, size_t bufferSize) override;
	size_t print(const char *value);
	void onConnect(BLEServer *pServer) override;
	void onDisconnect(BLEServer *pServer) override;

	bool connected() const;

	BLEServer *Server;

	BLEAdvertising *pAdvertising;
	//BLESecurity *pSecurity;

	//Services
	BLEService *SerialService;

	//Serial Characteristics
	BLECharacteristic *TxCharacteristic;
	BLECharacteristic *RxCharacteristic;

	bool enableLed = false;
	int ledPin = 13;
protected:
	size_t transmitBufferLength;

private:
	ByteRingBuffer<RX_BUFFER_SIZE> receiveBuffer;
	size_t numAvailableLines;

	uint8_t transmitBuffer[BLE_BUFFER_SIZE];

	void SetupSerialService();

	uint16_t peerMTU;
	uint16_t maxTransferSize = BLE_BUFFER_SIZE;

};
