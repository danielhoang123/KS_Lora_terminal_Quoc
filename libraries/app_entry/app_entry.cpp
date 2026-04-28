#include "app_entry.h"
#include "App.h"

static App app;

void app_setup() {
    app.setup();
}

void app_loop() {
    app.loop();
}