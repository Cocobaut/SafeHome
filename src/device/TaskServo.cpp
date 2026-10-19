#include "TaskServo.h"

// 360-degree (continuous rotation) servos: the angle value sets speed and
// direction, not position: 0 = full speed one way, 180 = full speed the other
// way, 90 (1500us) = stop. A latch moves by spinning for SERVO_MOVE_MS, then
// goes back to 90.
//
// PWM is driven straight from LEDC (one channel per servo) instead of the
// ESP32Servo library: on ESP32-S3, ESP32Servo 3.2.x puts the servos on MCPWM
// and routes the 3rd one (MCPWM timer 1) to the MCPWM0A signal, i.e. the same
// output as servo 1. That made the D10 servo only follow servo 1.

#define SERVO_PWM_FREQ      50      // 20ms period
#define SERVO_PWM_BITS      14
#define SERVO_PERIOD_US     20000UL
#define SERVO_MIN_US        500     // angle 0
#define SERVO_MAX_US        2500    // angle 180 (so 90 = 1500us, neutral)

static const uint8_t servoPins[SERVO_COUNT] = {PIN_SERVO_1, PIN_SERVO_2, PIN_SERVO_3};

// Spin angle used to OPEN each latch; closing spins the opposite way (180 - x)
//   Chot 1 (D11): Mo = 180, Dong = 0
//   Chot 2 (D12): Mo = 180, Dong = 0
//   Chot 3 (D10): Mo = 0,   Dong = 180
static const uint8_t openAngle[SERVO_COUNT] = {180, 180, 0};

static bool servoOpen[SERVO_COUNT];
static bool moving[SERVO_COUNT];
static unsigned long moveStartedAt[SERVO_COUNT];
static unsigned long moveDuration[SERVO_COUNT];

static void servoWriteAngle(uint8_t i, uint8_t angle)
{
    uint32_t us = SERVO_MIN_US + (uint32_t)angle * (SERVO_MAX_US - SERVO_MIN_US) / 180;
    uint32_t duty = us * (1UL << SERVO_PWM_BITS) / SERVO_PERIOD_US;
    ledcWrite(i, duty);  // LEDC channel i belongs to servo i
}

static uint8_t getSpinAngle(uint8_t i, bool open)
{
    return open ? openAngle[i] : 180 - openAngle[i];
}

static void startMove(uint8_t i, bool open)
{
    unsigned long now = millis();
    unsigned long duration = SERVO_MOVE_MS;
    // Reversing mid-move: spin back only as long as it already moved
    if (moving[i]) duration = now - moveStartedAt[i];

    uint8_t angle = getSpinAngle(i, open);
    servoWriteAngle(i, angle);

    moving[i] = true;
    moveStartedAt[i] = now;
    moveDuration[i] = duration;
    servoOpen[i] = open;
    Serial.printf("[Servo] Chot %d (GPIO%d): %s (quay goc %d)\n",
                  i + 1, servoPins[i], open ? "MO" : "DONG", angle);
}

static void stopMove(uint8_t i)
{
    servoWriteAngle(i, SERVO_STOP_ANGLE);
    moving[i] = false;
}

// force = true runs the move even if the latch is already in that state
// (admin uses it to re-sync a latch whose real position is off)
void servoSet(uint8_t index, bool open, bool force)
{
    if (index >= SERVO_COUNT) return;
    if (!force && servoOpen[index] == open) return;
    startMove(index, open);
}

void servoSetAll(bool open)
{
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        servoSet(i, open);
    }
}

bool servoIsOpen(uint8_t index)
{
    return index < SERVO_COUNT && servoOpen[index];
}

bool servoIsMoving(uint8_t index)
{
    return index < SERVO_COUNT && moving[index];
}

bool servoAnyOpen()
{
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        if (servoOpen[i]) return true;
    }
    return false;
}

void loopServo()
{
    unsigned long now = millis();
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        if (moving[i] && now - moveStartedAt[i] >= moveDuration[i]) stopMove(i);
    }
}

void initServo()
{
    // Latches are assumed closed at power-on, so nothing spins on boot
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        if (ledcSetup(i, SERVO_PWM_FREQ, SERVO_PWM_BITS) == 0) {
            Serial.printf("[Servo] LOI: khong setup duoc PWM chot %d (GPIO%d)\n", i + 1, servoPins[i]);
        }
        ledcAttachPin(servoPins[i], i);
        servoWriteAngle(i, SERVO_STOP_ANGLE);
        servoOpen[i] = false;
        moving[i] = false;
    }
}
