#include "App.h"
#include <WiFi.h>
#include "index_html.h"

void App::setup() {
    Serial.begin(115200);

    setupWiFi();
    setupServer();
    lora.begin();
}

void App::loop() {
    server.handleClient();

    // nhận LoRa
    if (lora.receive()) {
        String log = "[RX] " + lora.getMsg() +
                     " [RSSI] " + String(lora.getRSSI()) + "dBm" +
                     " [SNR] " + String(lora.getSNR(), 2) + "dB" +
                     " [RTT] " + String(lora.getRTT()) + "ms";

        logger.set(log);

        Serial.println(log);
    }
}

void App::setupWiFi() {
    WiFi.softAP("LoRa Terminal", "12345678");
    Serial.println("AP Started");
    Serial.println(WiFi.softAPIP());
}

void App::setupServer() {

    server.on("/", [this]() {
        server.send(200, "text/html", index_html);
    });

    server.on("/send", [this]() {
        handleSend();
    });

    server.on("/data", [this]() {
        handleData();
    });

    server.on("/status", [this]() {
        handleStatus();
    });

    server.begin();
}

void App::handleSend() {
    String msg = server.arg("msg");

    if (msg.length()) {

        lora.send(msg);
        
        String timeStr = getTimeString();   // ✅ dùng chung
        String log = "[TX] " + timeStr + " | " + msg;

        logger.set(log);

        Serial.println(log);
    }

    server.send(200, "text/plain", "OK");
}

void App::handleData() {
    if (logger.available()) {
        String json = "{\"log\":\"" + logger.get() + "\"}";
        server.send(200, "application/json", json);
    } else {
        server.send(200, "application/json", "{\"log\":\"\"}");
    }
}

void App::handleStatus() {
    String json = "{";
    json += "\"rtt\":" + String(lora.getRTT()) + ",";
    json += "\"rssi\":" + String(lora.getRSSI()) + ",";
    json += "\"snr\":" + String(lora.getSNR(), 2) + ",";
    json += "\"msg\":\"" + lora.getMsg() + "\"";
    json += "}";

    server.send(200, "application/json", json);
}