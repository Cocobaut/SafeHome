#include "TaskUltrasonic.h"

// Two sensors are pinged alternately (no crosstalk). The echo pulse is measured
// by a GPIO interrupt, so nothing blocks like pulseIn() does.
#define US_PING_INTERVAL_MS 60      // Between two consecutive pings
#define US_ECHO_TIMEOUT_MS  40      // No echo after this -> out of range
#define US_WINDOW           5       // Median filter window
#define US_CALIB_SAMPLES    15      // Samples used to build the baseline
#define US_CONFIRM_HITS     2       // Consecutive deviated medians to raise detection
#define US_CLEAR_MS         1000UL  // Must be back to baseline this long to clear

struct UltrasonicSensor {
    uint8_t trigPin;
    uint8_t echoPin;

    volatile uint32_t riseUs;
    volatile uint32_t pulseUs;
    volatile bool ready;

    float window[US_WINDOW];
    uint8_t windowIdx;
    uint8_t windowCount;

    float calib[US_CALIB_SAMPLES];
    uint8_t calibCount;

    float distance;
    float baseline;
    uint8_t hits;
    bool detected;
    bool clearing;
    unsigned long clearSince;
};

static UltrasonicSensor sensors[ULTRASONIC_COUNT] = {
    {PIN_US1_TRIG, PIN_US1_ECHO},
    {PIN_US2_TRIG, PIN_US2_ECHO},
};

static bool calibrating = true;
static uint8_t activeSensor = 0;
static bool waitingEcho = false;
static unsigned long pingAt = 0;

static void IRAM_ATTR echoISR(void *arg)
{
    UltrasonicSensor *s = static_cast<UltrasonicSensor *>(arg);
    uint32_t now = micros();
    if (gpio_get_level((gpio_num_t)s->echoPin)) {
        s->riseUs = now;
    } else {
        s->pulseUs = now - s->riseUs;
        s->ready = true;
    }
}

static float median(const float *values, uint8_t n)
{
    float sorted[US_CALIB_SAMPLES];
    for (uint8_t i = 0; i < n; i++) {
        float v = values[i];
        int8_t j = i - 1;
        while (j >= 0 && sorted[j] > v) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = v;
    }
    return (n % 2) ? sorted[n / 2] : (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0f;
}

static void ping(UltrasonicSensor &s)
{
    s.ready = false;
    digitalWrite(s.trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(s.trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(s.trigPin, LOW);
}

static void finishCalibrationIfDone()
{
    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        if (sensors[i].calibCount < US_CALIB_SAMPLES) return;
    }
    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        sensors[i].baseline = US_ALERT_DISTANCE_CM;
        Serial.printf("[Ultrasonic] Cam bien %d san sang (nguong bao: %.1f cm)\n", i + 1, US_ALERT_DISTANCE_CM);
    }
    calibrating = false;
}

static void processSample(UltrasonicSensor &s, float cm, unsigned long now)
{
    s.window[s.windowIdx] = cm;
    s.windowIdx = (s.windowIdx + 1) % US_WINDOW;
    if (s.windowCount < US_WINDOW) s.windowCount++;
    s.distance = median(s.window, s.windowCount);

    if (calibrating) {
        if (s.calibCount < US_CALIB_SAMPLES) s.calib[s.calibCount++] = cm;
        finishCalibrationIfDone();
        return;
    }

    // Nho hon hoac bang nguong US_ALERT_DISTANCE_CM (<= 10cm) thi bao dong
    bool triggered = (s.distance > 0.0f && s.distance <= US_ALERT_DISTANCE_CM);
    if (triggered) {
        s.clearing = false;
        if (s.hits < 255) s.hits++;
        if (s.hits >= US_CONFIRM_HITS) s.detected = true;
    } else {
        s.hits = 0;
        if (s.detected) {
            if (!s.clearing) {
                s.clearing = true;
                s.clearSince = now;
            } else if (now - s.clearSince >= US_CLEAR_MS) {
                s.detected = false;
                s.clearing = false;
            }
        }
    }
}

void ultrasonicRecalibrate()
{
    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        UltrasonicSensor &s = sensors[i];
        s.windowCount = 0;
        s.windowIdx = 0;
        s.calibCount = 0;
        s.hits = 0;
        s.detected = false;
        s.clearing = false;
    }
    calibrating = true;
    Serial.println("[Ultrasonic] Dang hieu chuan moc khoang cach...");
}

bool ultrasonicCalibrating()
{
    return calibrating;
}

float ultrasonicDistance(uint8_t index)
{
    return index < ULTRASONIC_COUNT ? sensors[index].distance : 0;
}

float ultrasonicBaseline(uint8_t index)
{
    return index < ULTRASONIC_COUNT ? sensors[index].baseline : 0;
}

bool ultrasonicDetected(uint8_t index)
{
    return index < ULTRASONIC_COUNT && !calibrating && sensors[index].detected;
}

void loopUltrasonic()
{
    unsigned long now = millis();

    if (waitingEcho) {
        UltrasonicSensor &s = sensors[activeSensor];
        if (!s.ready && now - pingAt < US_ECHO_TIMEOUT_MS) return;

        // Nothing in range is treated as max distance, so an empty balcony
        // gets a valid baseline and anything appearing is still a deviation.
        float cm = US_MAX_DISTANCE_CM;
        if (s.ready) {
            float measured = s.pulseUs / 58.0f;
            if (measured >= US_MIN_DISTANCE_CM && measured <= US_MAX_DISTANCE_CM) cm = measured;
        }
        processSample(s, cm, now);

        waitingEcho = false;
        activeSensor = (activeSensor + 1) % ULTRASONIC_COUNT;
    }

    if (now - pingAt >= US_PING_INTERVAL_MS) {
        ping(sensors[activeSensor]);
        pingAt = now;
        waitingEcho = true;
    }
}

void initUltrasonic()
{
    for (uint8_t i = 0; i < ULTRASONIC_COUNT; i++) {
        pinMode(sensors[i].trigPin, OUTPUT);
        digitalWrite(sensors[i].trigPin, LOW);
        pinMode(sensors[i].echoPin, INPUT);
        attachInterruptArg(digitalPinToInterrupt(sensors[i].echoPin), echoISR, &sensors[i], CHANGE);
    }
    ultrasonicRecalibrate();
}
