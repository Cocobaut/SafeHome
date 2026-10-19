#include "globals.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("YOLO UNO SECURITY starting...");

    initSensor();
    initSecurity();
    delay(500);

    // Initialize Access Point and web server
    initAcceptPoint();
    initWebserver();
}

void loop()
{
    loopSensor();
    loopSecurity();
    loopWebserver();
}
