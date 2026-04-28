#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H


#include <LoRa.h>
#include "TimeUtils.h"

class LoRaManager {
public:
    void begin();
    void send(String msg);
    bool receive();

    unsigned long getRTT();
    int getRSSI();
    float getSNR();
    String getMsg();

private:
    unsigned long lastRTT = 0;
    int lastRSSI = 0;
    float lastSNR = 0;
    String lastMsg = "";

};

#endif