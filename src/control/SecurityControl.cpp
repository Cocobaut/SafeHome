#include "SecurityControl.h"

// Timed unlock (web password / inside button)
static bool unlockActive = false;
static unsigned long unlockStartedAt = 0;

// Gas emergency: all latches open while gas is high
static bool gasEmergency = false;
static bool gasClearing = false;
static unsigned long gasClearSince = 0;

// Intrusion
static bool doorAlarm = false;  // Active while the forced-open door stays open
static bool usAlarm[ULTRASONIC_COUNT] = {false, false};
static bool usPresent[ULTRASONIC_COUNT] = {false, false};
static unsigned long usPresentSince[ULTRASONIC_COUNT] = {0, 0};

// Pepper spray motor
static bool sprayActive = false;
static unsigned long sprayStartedAt = 0;

// Admin overrides
static bool manualBuzzer = false;
static bool manualRelay = false;

static void triggerSpray(const char *reason)
{
    // People must be able to escape during a gas leak
    if (gasEmergency) return;
    sprayActive = true;
    sprayStartedAt = millis();
    Serial.printf("[Security] XIT CAY: %s\n", reason);
}

static void relockIfIdle()
{
    if (!unlockActive && !gasEmergency) servoSetAll(false);
}

void securityRequestUnlock()
{
    unlockActive = true;
    unlockStartedAt = millis();
    doorAlarm = false;
    servoSetAll(true);
    Serial.println("[Security] Mo khoa 10 giay");
}

void securityResetAlarms()
{
    doorAlarm = false;
    sprayActive = false;
    manualBuzzer = false;
    manualRelay = false;
    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        usPresentSince[i] = millis();
    }
    Serial.println("[Security] Reset canh bao");
}

void securitySetManualBuzzer(bool on)
{
    manualBuzzer = on;
    // Turning it off also silences a latched door alarm (gas alarm keeps sounding)
    if (!on) doorAlarm = false;
}

void securitySetManualRelay(bool on)
{
    manualRelay = on;
    if (!on) sprayActive = false;
}

void securityAdminServo(uint8_t index, bool open)
{
    servoSet(index, open, true);
}

bool securityUnlockActive()
{
    return unlockActive;
}

unsigned long securityUnlockRemainingMs()
{
    if (!unlockActive) return 0;
    unsigned long elapsed = millis() - unlockStartedAt;
    return elapsed >= UNLOCK_DURATION_MS ? 0 : UNLOCK_DURATION_MS - elapsed;
}

bool securityGasEmergency()
{
    return gasEmergency;
}

bool securityDoorAlarm()
{
    return doorAlarm;
}

bool securityUltrasonicAlarm(uint8_t index)
{
    return index < ULTRASONIC_COUNT && usAlarm[index];
}

bool securityUltrasonicSuppressed()
{
    return unlockActive;
}

static void handleGas(unsigned long now)
{
    if (gasOverThreshold()) {
        gasClearing = false;
        if (!gasEmergency) {
            gasEmergency = true;
            sprayActive = false;
            servoSetAll(true);
            Serial.println("[Security] RO RI GAS - mo het chot thoat hiem");
        }
        return;
    }

    if (!gasEmergency) return;
    if (!gasClearing) {
        gasClearing = true;
        gasClearSince = now;
    } else if (now - gasClearSince >= GAS_CLEAR_HOLD_MS) {
        gasEmergency = false;
        gasClearing = false;
        Serial.println("[Security] Gas tro lai binh thuong");
        relockIfIdle();
    }
}

static void handleUnlockWindow(unsigned long now)
{
    if (unlockActive && now - unlockStartedAt >= UNLOCK_DURATION_MS) {
        unlockActive = false;
        Serial.println("[Security] Het 10 giay - tu dong khoa");
        relockIfIdle();
    }
}

static void handleDoor()
{
    // Door back in place: clear the intrusion together with the motor and buzzer
    if (doorAlarm && !doorIsOpen()) {
        doorAlarm = false;
        sprayActive = false;
        Serial.println("[Security] Cua dong lai - tat canh bao dot nhap");
    }

    if (!doorConsumeOpenedEvent()) return;

    bool authorized = unlockActive || gasEmergency || servoAnyOpen();
    if (!authorized) {
        doorAlarm = true;
        Serial.println("[Security] CUA BI MO TRAI PHEP");
        triggerSpray("cua bi mo trai phep");
    }
}

static void handleUltrasonic(unsigned long now)
{
    bool suppressed = securityUltrasonicSuppressed();

    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        bool detected = !suppressed && ultrasonicDetected(i);
        if (detected && !usAlarm[i]) {
            Serial.printf("[Security] Ultrasonic %d: phat hien xam nhap\n", i + 1);
        }
        usAlarm[i] = detected;

        if (!detected) {
            usPresent[i] = false;
            continue;
        }
        if (!usPresent[i]) {
            usPresent[i] = true;
            usPresentSince[i] = now;
        } else if (now - usPresentSince[i] >= US_SPRAY_AFTER_MS) {
            triggerSpray(i == 0 ? "ban cong truoc > 10s" : "ban cong sau > 10s");
            usPresentSince[i] = now;  // Spray again only after another 10s
        }
    }
}

static void handleButtons()
{
    if (buttonConsumeShortPress(BUTTON_OPEN)) securityRequestUnlock();
}

static void updateOutputs(unsigned long now)
{
    if (sprayActive && now - sprayStartedAt >= SPRAY_DURATION_MS) sprayActive = false;
    relayWrite(manualRelay || sprayActive);

    if (manualBuzzer || gasEmergency) {
        buzzerSetMode(BUZZER_CONTINUOUS);
    } else if (doorAlarm || usAlarm[0] || usAlarm[1]) {
        buzzerSetMode(BUZZER_BEEP);
    } else {
        buzzerSetMode(BUZZER_OFF);
    }
}

void loopSecurity()
{
    unsigned long now = millis();

    handleButtons();
    handleGas(now);
    handleUnlockWindow(now);
    handleDoor();
    handleUltrasonic(now);
    updateOutputs(now);
}

void initSecurity()
{
    // Normal / night mode: always locked
    servoSetAll(false);
}
