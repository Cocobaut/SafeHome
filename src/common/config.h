#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ================= PINOUT YOLO UNO (ESP32-S3) =================
// Servo chot cua
#define PIN_SERVO_1         38  // D11
#define PIN_SERVO_2         47  // D12
#define PIN_SERVO_3         21  // D10 (Chuyen tu A1 sang D10)

// Cam bien sieu am HC-SR04
#define PIN_US1_TRIG        5   // D2 - Ban cong truoc
#define PIN_US1_ECHO        6   // D3
#define PIN_US2_TRIG        7   // D4 - Ban cong sau
#define PIN_US2_ECHO        8   // D5

// Cam bien tu cua (INPUT_PULLUP)
#define PIN_DOOR            9   // D6

// Cam bien khi gas MQ-2 (ADC1_CH0)
#define PIN_GAS             1   // A0

// Coi bao dong (active buzzer)
#define PIN_BUZZER          10  // D7

// Relay motor xit cay (active LOW)
#define PIN_RELAY           17  // D8

// Nut bam (INPUT_PULLUP) - chi giu 1 nut o D9
#define PIN_BUTTON_1        18  // D9  - Nut mo cua tu ben trong

// ================= MUC LOGIC =================
#define BUZZER_ON_LEVEL     HIGH
#define RELAY_ON_LEVEL      LOW
// Cong tac tu thuong mo noi GND: nam cham ap sat -> LOW (dong), tach ra -> HIGH (mo)
#define DOOR_OPEN_LEVEL     HIGH

// ================= SERVO =================
// Servo 360 do: gia tri write() la toc do/chieu quay, 90 = dung
#define SERVO_STOP_ANGLE    90
#define SERVO_MOVE_MS       150UL   // Quay 0.15s roi ve 90 (dung)
#define SERVO_COUNT         3

// ================= THOI GIAN =================
#define UNLOCK_DURATION_MS      10000UL     // Mo khoa bang mat khau
#define US_SPRAY_AFTER_MS       10000UL     // Phat hien lien tuc -> xit cay
#define SPRAY_DURATION_MS       3000UL      // Thoi gian chay motor xit moi lan
#define ADMIN_SESSION_MS        600000UL    // Phien admin 10 phut
#define GAS_CLEAR_HOLD_MS       10000UL     // Gas on dinh duoi nguong bao lau thi khoa lai
#define GAS_WARMUP_MS           30000UL     // Thoi gian lam nong MQ-2, bo qua canh bao
#define LOGIN_MAX_FAILS         5
#define LOGIN_LOCKOUT_MS        30000UL

// ================= NGUONG CAM BIEN =================
#define GAS_THRESHOLD           2000        // ADC 12-bit (0..4095)
#define GAS_HYSTERESIS          150
#define US_ALERT_DISTANCE_CM    10.0f       // Khoang cach toi da de bao dong (nho hon/bang 10cm se bao dong)
#define US_MIN_DISTANCE_CM      0.5f
#define US_MAX_DISTANCE_CM      400.0f

// ================= MANG / MAT KHAU =================
extern const char AP_SSID[];
extern const char AP_PASSWORD[];
extern const char USER_PASSWORD[];
extern const char ADMIN_PASSWORD[];

#endif
