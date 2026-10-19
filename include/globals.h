#ifndef GLOBALS_H
#define GLOBALS_H

// include libraries
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Pinout, passwords, thresholds
#include "common/config.h"

// Connect
#include "../src/connect/acceptPoint.h"
#include "../src/connect/Webserver.h"

// Device
#include "../src/device/TaskServo.h"
#include "../src/device/TaskUltrasonic.h"
#include "../src/device/TaskDoorSensor.h"
#include "../src/device/TaskGasSensor.h"
#include "../src/device/TaskBuzzer.h"
#include "../src/device/TaskRelay.h"
#include "../src/device/TaskButton.h"
#include "../src/device/initSensor.h"

// Control
#include "../src/control/SecurityControl.h"

#endif
