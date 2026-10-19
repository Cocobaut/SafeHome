#include "TaskGasSensor.h"

#define GAS_READ_INTERVAL_MS 100
#define GAS_EMA_ALPHA        0.2f

static int rawValue = 0;
static float filteredValue = 0;
static bool overThreshold = false;
static unsigned long startedAt = 0;
static unsigned long lastRead = 0;

int gasRawValue()
{
    return rawValue;
}

int gasFilteredValue()
{
    return (int)filteredValue;
}

bool gasWarmingUp()
{
    return millis() - startedAt < GAS_WARMUP_MS;
}

bool gasOverThreshold()
{
    return overThreshold;
}

void loopGasSensor()
{
    unsigned long now = millis();
    if (now - lastRead < GAS_READ_INTERVAL_MS) return;
    lastRead = now;

    rawValue = analogRead(PIN_GAS);
    filteredValue += GAS_EMA_ALPHA * (rawValue - filteredValue);

    // MQ-2 reads high while its heater warms up: ignore that period
    if (gasWarmingUp()) {
        overThreshold = false;
        return;
    }

    if (!overThreshold && filteredValue >= GAS_THRESHOLD) {
        overThreshold = true;
    } else if (overThreshold && filteredValue < GAS_THRESHOLD - GAS_HYSTERESIS) {
        overThreshold = false;
    }
}

void initGasSensor()
{
    analogReadResolution(12);
    rawValue = analogRead(PIN_GAS);
    filteredValue = rawValue;
    startedAt = millis();
}
