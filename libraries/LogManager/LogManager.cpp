#include "LogManager.h"

void LogManager::set(String log) {
    logLine = log;
    hasNew = true;
}

String LogManager::get() {
    hasNew = false;
    return logLine;
}

bool LogManager::available() {
    return hasNew;
}