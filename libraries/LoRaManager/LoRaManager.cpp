#include "LoRaManager.h"



void LoRaManager::begin() {
    LoRa.setPins(5, 13, 14);

    while (!LoRa.begin(915E6)) {
        delay(500);
    }

    LoRa.setSyncWord(0xF3);
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(8);
    LoRa.setPreambleLength(12);
    LoRa.enableCrc();

    Serial.println("LoRa OK");
}

void LoRaManager::send(String msg) {
    
    unsigned long now = millis();
    String timeStr = getTimeString();
    String fullMsg = timeStr + "|" + String(now) + "|" + msg;

    LoRa.beginPacket();
    LoRa.print(fullMsg);
    LoRa.endPacket();
}

bool LoRaManager::receive() {
    int packetSize = LoRa.parsePacket();
    if (!packetSize) return false;

    String received = "";

    while (LoRa.available()) {
        received += (char)LoRa.read();
    }

    int first = received.indexOf('|');
    int second = received.indexOf('|', first + 1);

    if (first == -1 || second == -1) return false;

    unsigned long sentMillis =
        received.substring(first + 1, second).toInt();

    if (sentMillis == 0) return false;

    lastMsg = received.substring(second + 1);
    lastRTT = millis() - sentMillis;
    lastRSSI = LoRa.packetRssi();
    lastSNR = LoRa.packetSnr();

    return true;
}

unsigned long LoRaManager::getRTT() { return lastRTT; }
int LoRaManager::getRSSI() { return lastRSSI; }
float LoRaManager::getSNR() { return lastSNR; }
String LoRaManager::getMsg() { return lastMsg; }