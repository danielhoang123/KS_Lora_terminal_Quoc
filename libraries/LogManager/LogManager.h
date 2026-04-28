#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "Arduino.h"

class LogManager {
public:
    void set(String log);
    String get();
    bool available();

private:
    String logLine = "";
    bool hasNew = false;
};

#endif