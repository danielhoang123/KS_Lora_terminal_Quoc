#ifndef APP_H
#define APP_H

#include <WebServer.h>
#include "LoRaManager.h"
#include "LogManager.h"
#include "TimeUtils.h"

class App {
public:
    void setup();
    void loop();

private:
    WebServer server = WebServer(80);
    LoRaManager lora;
    LogManager logger;

    void setupWiFi();
    void setupServer();

    void handleSend();
    void handleData();
    void handleStatus();
};

#endif