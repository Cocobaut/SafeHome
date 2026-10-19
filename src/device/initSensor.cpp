#include "initSensor.h"

void initSensor() {
    // Actuators first so that locks/relay are in a safe state right after boot
    initRelay();
    initBuzzer();
    initServo();

    initUltrasonic();
    initDoorSensor();
    initGasSensor();
    initButtons();
}

void loopSensor() {
    loopUltrasonic();
    loopDoorSensor();
    loopGasSensor();
    loopButtons();
    loopBuzzer();
    loopServo();
}
