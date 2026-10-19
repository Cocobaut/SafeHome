#include "Webserver.h"
#include "user_page.h"
#include "admin_page.h"

static WebServer server(80);

// Admin session (one admin at a time, absolute 10-minute lifetime)
static String adminToken;
static unsigned long adminLoginAt = 0;

// Brute-force protection
struct LoginGuard {
    uint8_t fails;
    bool locked;
    unsigned long lockedAt;
};
static LoginGuard userGuard = {0, false, 0};
static LoginGuard adminGuard = {0, false, 0};

static unsigned long guardRemainingMs(LoginGuard &g)
{
    if (!g.locked) return 0;
    unsigned long elapsed = millis() - g.lockedAt;
    if (elapsed >= LOGIN_LOCKOUT_MS) {
        g.locked = false;
        return 0;
    }
    return LOGIN_LOCKOUT_MS - elapsed;
}

static void guardFail(LoginGuard &g)
{
    if (++g.fails >= LOGIN_MAX_FAILS) {
        g.fails = 0;
        g.locked = true;
        g.lockedAt = millis();
    }
}

static void sendJson(int code, const String &json)
{
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json", json);
}

static void sendLockedOut(unsigned long remainingMs)
{
    sendJson(429, "{\"ok\":false,\"err\":\"locked\",\"wait\":" + String((remainingMs + 999) / 1000) + "}");
}

static unsigned long adminSessionRemainingMs()
{
    if (adminToken.isEmpty()) return 0;
    unsigned long elapsed = millis() - adminLoginAt;
    if (elapsed >= ADMIN_SESSION_MS) {
        adminToken = "";
        return 0;
    }
    return ADMIN_SESSION_MS - elapsed;
}

static bool requireAdmin()
{
    if (adminSessionRemainingMs() > 0 && server.arg("token") == adminToken) return true;
    sendJson(401, "{\"ok\":false,\"err\":\"session\"}");
    return false;
}

static String newToken()
{
    char buf[17];
    snprintf(buf, sizeof(buf), "%08lx%08lx", (unsigned long)esp_random(), (unsigned long)esp_random());
    return String(buf);
}

static const char *jbool(bool v)
{
    return v ? "true" : "false";
}

static String buildStatusJson(bool admin)
{
    char buf[768];
    int n = snprintf(buf, sizeof(buf),
        "{\"locked\":%s,\"servo\":[%d,%d,%d],\"unlock\":%lu,"
        "\"cal\":%s,\"sup\":%s,"
        "\"us\":[{\"d\":%.1f,\"b\":%.1f,\"a\":%s},{\"d\":%.1f,\"b\":%.1f,\"a\":%s}],"
        "\"door\":{\"open\":%s,\"a\":%s},"
        "\"gas\":{\"v\":%d,\"th\":%d,\"a\":%s,\"warm\":%s},"
        "\"buzzer\":%s,\"relay\":%s",
        jbool(!servoAnyOpen()), servoIsOpen(0), servoIsOpen(1), servoIsOpen(2),
        securityUnlockRemainingMs(),
        jbool(ultrasonicCalibrating()), jbool(securityUltrasonicSuppressed()),
        ultrasonicDistance(0), ultrasonicBaseline(0), jbool(securityUltrasonicAlarm(0)),
        ultrasonicDistance(1), ultrasonicBaseline(1), jbool(securityUltrasonicAlarm(1)),
        jbool(doorIsOpen()), jbool(securityDoorAlarm()),
        gasFilteredValue(), GAS_THRESHOLD, jbool(securityGasEmergency()), jbool(gasWarmingUp()),
        jbool(buzzerIsOn()), jbool(relayIsOn()));

    if (admin) {
        n += snprintf(buf + n, sizeof(buf) - n, ",\"raw\":%d,\"session\":%lu,\"mv\":[%d,%d,%d]",
                      gasRawValue(), adminSessionRemainingMs(),
                      servoIsMoving(0), servoIsMoving(1), servoIsMoving(2));
    }
    snprintf(buf + n, sizeof(buf) - n, "}");
    return String(buf);
}

// ================= USER =================
static void handleUserPage()
{
    server.send(200, "text/html; charset=utf-8", USER_PAGE);
}

static void handleStatus()
{
    sendJson(200, buildStatusJson(false));
}

static void handleUnlock()
{
    unsigned long wait = guardRemainingMs(userGuard);
    if (wait) return sendLockedOut(wait);

    if (server.arg("password") != USER_PASSWORD) {
        guardFail(userGuard);
        return sendJson(403, "{\"ok\":false,\"err\":\"wrong\"}");
    }
    userGuard.fails = 0;
    securityRequestUnlock();
    sendJson(200, "{\"ok\":true,\"ms\":" + String(UNLOCK_DURATION_MS) + "}");
}

// ================= ADMIN =================
static void handleAdminPage()
{
    server.send(200, "text/html; charset=utf-8", ADMIN_PAGE);
}

static void handleAdminLogin()
{
    unsigned long wait = guardRemainingMs(adminGuard);
    if (wait) return sendLockedOut(wait);

    if (server.arg("password") != ADMIN_PASSWORD) {
        guardFail(adminGuard);
        return sendJson(403, "{\"ok\":false,\"err\":\"wrong\"}");
    }
    adminGuard.fails = 0;
    adminToken = newToken();
    adminLoginAt = millis();
    Serial.println("[Web] Admin dang nhap");
    sendJson(200, "{\"ok\":true,\"token\":\"" + adminToken + "\",\"session\":" + String(ADMIN_SESSION_MS) + "}");
}

static void handleAdminLogout()
{
    if (server.arg("token") == adminToken) adminToken = "";
    sendJson(200, "{\"ok\":true}");
}

static void handleAdminStatus()
{
    if (!requireAdmin()) return;
    sendJson(200, buildStatusJson(true));
}

static void handleAdminServo()
{
    if (!requireAdmin()) return;
    int id = server.arg("id").toInt();
    String action = server.arg("action");
    if (id < 1 || id > SERVO_COUNT || (action != "open" && action != "close")) {
        return sendJson(400, "{\"ok\":false,\"err\":\"param\"}");
    }
    securityAdminServo(id - 1, action == "open");
    sendJson(200, "{\"ok\":true}");
}

static void handleAdminBuzzer()
{
    if (!requireAdmin()) return;
    securitySetManualBuzzer(server.arg("state") == "on");
    sendJson(200, "{\"ok\":true}");
}

static void handleAdminRelay()
{
    if (!requireAdmin()) return;
    securitySetManualRelay(server.arg("state") == "on");
    sendJson(200, "{\"ok\":true}");
}

static void handleAdminCalibrate()
{
    if (!requireAdmin()) return;
    ultrasonicRecalibrate();
    sendJson(200, "{\"ok\":true}");
}

static void handleAdminReset()
{
    if (!requireAdmin()) return;
    securityResetAlarms();
    sendJson(200, "{\"ok\":true}");
}

static void handleNotFound()
{
    server.send(404, "text/plain", "Not found");
}

void initWebserver()
{
    server.on("/", HTTP_GET, handleUserPage);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/unlock", HTTP_POST, handleUnlock);

    server.on("/admin", HTTP_GET, handleAdminPage);
    server.on("/admin/login", HTTP_POST, handleAdminLogin);
    server.on("/admin/logout", HTTP_POST, handleAdminLogout);
    server.on("/admin/status", HTTP_POST, handleAdminStatus);
    server.on("/admin/servo", HTTP_POST, handleAdminServo);
    server.on("/admin/buzzer", HTTP_POST, handleAdminBuzzer);
    server.on("/admin/relay", HTTP_POST, handleAdminRelay);
    server.on("/admin/calibrate", HTTP_POST, handleAdminCalibrate);
    server.on("/admin/reset", HTTP_POST, handleAdminReset);

    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started on port 80");
}

void loopWebserver()
{
    server.handleClient();
}
