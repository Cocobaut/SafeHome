#include "acceptPoint.h"

void initAcceptPoint() {
    Serial.print("Setting AP (Access Point)...");
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
}
