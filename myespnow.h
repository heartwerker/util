#pragma once
#include <Arduino.h>

#if ESP8266
#include <espnow.h>
#include <ESP8266WiFi.h>
#elif ESP32 // https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
#include <esp_now.h>
#include <WiFi.h>
esp_now_peer_info_t peerInfo;
#endif

// ========================= Usuage =========================

// run once with dummy mac address to get the MAC address of the device
// then set:
// uint8_t MAC_RX[6] = {0x11, 0x22, 0x33, 0x2F, 0x58, 0x6E};

// #if ENABLE_ESPNOW
// void ESPNOW_receiveBytes(uint8_t *data, uint8_t len)
// {
//     // memcpy(&msg_from_remote, data, len);
//     Serial.printf("ESPNOW_receiveBytes(%d): ", len);
// }
// #endif

// void setup()
// {
//   ESPNOW_Init(ESPNOW_receiveBytes, MAC_RX);
//   [...]
// }

// later: 
// ESPNOW_send(.....)

// ========================= PROTOCOL =========================
typedef struct message_generic
{
    uint8_t index = 0;
    float value = 0;
} message_generic;

message_generic msg;

//================================================================

#define NUM_MAX_TARGETS 5
uint8_t *_targets[NUM_MAX_TARGETS] = {nullptr, nullptr, nullptr, nullptr, nullptr};

// typedef void (*esp_now_send_cb_t)(const uint8_t *mac_addr, esp_now_send_status_t status);
//================================================================
#if ESP8266
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
#if 1
    printf("Delivery %s", (sendStatus == 0) ? "success! \n" : "FAIL!!! \n");
#endif
}
#elif ESP32
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
#if 1
    printf("Delivery %s", (status == ESP_NOW_SEND_SUCCESS) ? "success! \n" : "FAIL!!! \n");
#endif
}
#endif

typedef void (*ESPNOW_RX_data_callback)(uint8_t *data, uint8_t len);
ESPNOW_RX_data_callback _receiveBytes = nullptr;

#if ESP8266
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
    if (_receiveBytes != nullptr)
        _receiveBytes(incomingData, len);
}
#elif ESP32
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    if (_receiveBytes != nullptr)
        _receiveBytes((uint8_t *)incomingData, len);
}
#endif

void ESPNOW_registerReceiver(unsigned char *address)
{
#if ESP32
    // Register peer
    memcpy(peerInfo.peer_addr, address, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
#elif ESP8266

#endif
}

void ESPNOW_send_generic(uint8_t *target, int index, float value);
void ESPNOW_send_generic(int index, float value);

void ESPNOW_Init(ESPNOW_RX_data_callback callback, uint8_t *target_addresses[], int num_targets)
{
    _receiveBytes = callback;

    for (int i = 0; i < num_targets; i++)
        _targets[i] = target_addresses[i];

#if 0 // todo is this necessary ?
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_AP_STA);
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
#else // old working esp8266 espnow_protocol.h
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
#endif

    if (esp_now_init() != 0)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

#if ESP8266
    // Set ESP-NOW Role
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
#endif

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    if (num_targets != 0)
        esp_now_register_send_cb(OnDataSent);

    for (int t = 0; t < num_targets; t++)
        if (_targets[t] != nullptr)
        {
#if ESP8266
            esp_now_add_peer(_targets[t], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
            // esp_now_add_peer(_targets[t], ESP_NOW_ROLE_COMBO, 1 + t, NULL, 0);
#elif ESP32
            ESPNOW_registerReceiver(_targets[t]);
#endif
            Serial.printf("ESPNOW_Init:: registered target with MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", _targets[t][0], _targets[t][1], _targets[t][2], _targets[t][3], _targets[t][4], _targets[t][5]);
        }

    if (_receiveBytes != nullptr)
    {
        // Register for a callback function that will be called when data is received
        esp_now_register_recv_cb(OnDataRecv);
    }

    Serial.println("ESPNOW_Init() done");

    printf("ESPNOW_init:: this device's MAC = %s\n", WiFi.macAddress().c_str());

    if (num_targets != 0)
    {

        delay(1000);
        for (int t = 0; t < num_targets; t++)
            if (_targets[t] != nullptr)
            {
                Serial.printf("ESPNOW_Init:: sending test message to target with id: %d\n", t);
                ESPNOW_send_generic(_targets[t], 0, 0);
                delay(1000);
            }
    }
}

void ESPNOW_Init(ESPNOW_RX_data_callback callback, uint8_t *target_mac_address)
{
    if (target_mac_address == nullptr)
        ESPNOW_Init(callback, nullptr, 0);
    else
        ESPNOW_Init(callback, &target_mac_address, 1);
}

void ESPNOW_sendBytes(uint8_t *data, uint8_t len)
{
    if (_targets[0] != nullptr)
        esp_now_send(_targets[0], data, len);
}

void ESPNOW_send_generic(uint8_t *target, int index, float value)
{
    message_generic msg;
    msg.index = index;
    msg.value = constrain(value, -1, 1);
    esp_now_send(target, (uint8_t *)&msg, sizeof(msg));
#if 0
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", target[0], target[1], target[2], target[3], target[4], target[5]);
    Serial.printf("ESPNOW_send_generic(%s, %d, %f)\n", macStr, index, value);
#endif
}

void ESPNOW_send_generic(int index, float value)
{
    message_generic msg;
    msg.index = index;
    msg.value = value;
    if (_targets[0] != nullptr)
        esp_now_send(_targets[0], (uint8_t *)&msg, sizeof(msg));
}
