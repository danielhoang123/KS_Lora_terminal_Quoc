#include "TimeUtils.h"

String getTimeString() {
    unsigned long ms = millis();

    unsigned long totalSeconds = ms / 1000;
    unsigned int h = totalSeconds / 3600;
    unsigned int m = (totalSeconds % 3600) / 60;
    unsigned int s = totalSeconds % 60;

    char buf[9];
    sprintf(buf, "%02u:%02u:%02u", h, m, s);

    return String(buf);
}