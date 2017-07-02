#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>    //https://github.com/Links2004/arduinoWebSockets/issues/61
#include <Hash.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


// Transparent Serial Bridge code from Marcus https://github.com/Links2004/arduinoWebSockets/issues/61

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* host = "webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";


WebSocketsServer webSocket = WebSocketsServer(81);


WiFiManager wifiManager;
int loopCount = 0;
int resetCount = 0;
int RESET_PIN = 0; // = GPIO0 on nodeMCU
bool socketConnected = false;

#define SEND_SERIAL_TIME (50)

class SerialTerminal {
    public:
        void setup() {
            _lastRX = 0;
            resetBuffer();
            Serial.begin(115200);
        }

        void loop() {
            unsigned long t = millis();
            bool forceSend = false;

            size_t len = (_bufferWritePtr - &_buffer[0]);
            int free = (sizeof(_buffer) - len);

            int available = Serial.available();
            if(available > 0 && free > 0) {
                int readBytes = available;
                if(readBytes > free) {
                    readBytes = free;
                }
                readBytes = Serial.readBytes(_bufferWritePtr, readBytes);
                _bufferWritePtr += readBytes;
                _lastRX = t;
            }

            // check for data in buffer
            len = (_bufferWritePtr - &_buffer[0]);
            if(len >=  sizeof(_buffer)) {
                forceSend = true;
            }
            if(len > (WEBSOCKETS_MAX_HEADER_SIZE + 1)) {
                if(((t - _lastRX) > SEND_SERIAL_TIME) || forceSend) {
                    //Serial1.printf("broadcastBIN forceSend: %d\n", forceSend);
                    webSocket.broadcastTXT(&_buffer[0], (len - WEBSOCKETS_MAX_HEADER_SIZE), true);
                    resetBuffer();
                }
            }
        }


    protected:
        uint8_t _buffer[1460];
        uint8_t * _bufferWritePtr;
        unsigned long _lastRX;

        void resetBuffer() {
            // offset for adding Websocket header
            _bufferWritePtr = &_buffer[WEBSOCKETS_MAX_HEADER_SIZE];
            // addChar('T');
        }

        inline void addChar(char c) {
            *_bufferWritePtr = (uint8_t) c; // message type for Webinterface
            _bufferWritePtr++;
        }
};

SerialTerminal term;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    switch(type) {
        case WStype_DISCONNECTED:
            socketConnected = false;
//            Serial1.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            socketConnected = true;
//            Serial1.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
//            Serial1.printf("[%u] get Text: %s\n", num, payload);
            if(lenght > 0) {
                String command = String((const char *)payload);
                if (command.indexOf("resetWiFi") != -1){
                    webSocket.sendTXT(num, "Resetting WiFi settings...");
                    delay(500);
                    //wifiManager.resetSettings();
                    delay(100);
                    //ESP.restart();
                }
                Serial.write((const char *) (payload), (lenght));
            }
            break;
    }
}

void setup()
{
    // use Serial 1 for debug out
//    Serial1.begin(921600);
//    Serial1.setDebugOutput(true);

//    Serial1.println();
//    Serial1.println();
//    Serial1.println();

    for(uint8_t t = 4; t > 0; t--) {
//        Serial1.printf("[SETUP] BOOT WAIT %d...\n", t);
        delay(1000);
    }

//    Serial1.printf("[SETUP] HEAP: %d\n", ESP.getFreeHeap());

    pinMode(RESET_PIN, INPUT_PULLUP);

    //WiFiManager wifiManager;
    //wifiManager.resetSettings();    
        
    wifiManager.autoConnect("Emblaser2");

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    httpUpdater.setup(&httpServer, update_path, update_username, update_password);
    httpServer.begin();

    term.setup();

    // disable WiFi sleep for more performance
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
}


void loop()
{
    term.loop();
    webSocket.loop();
    httpServer.handleClient();

    if (socketConnected == false){
        //check every 10 loops
        if (loopCount > 100){
            loopCount = 0;
            // if GPIO0 is LOW for 10 times -> reset settings
            if (resetCount > 10){
                resetCount = 0;
                //wifiManager.resetSettings();
                delay(100);   
                //ESP.restart();
            } else {
                if (digitalRead(RESET_PIN) == LOW) {
                    resetCount++;
                } else {
                    resetCount = 0;
                }
            }
        } else {
            loopCount++;
        }
    }
}
