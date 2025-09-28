#include "NTPTimeSync.h"
#include "UnifiedLogger.h"

// NTP Server configuration for Mexico
const char* NTPTimeSync::NTP_SERVER_1 = "pool.ntp.org";
const char* NTPTimeSync::NTP_SERVER_2 = "time.nist.gov";
const char* NTPTimeSync::NTP_SERVER_3 = "mx.pool.ntp.org";
const char* NTPTimeSync::TIMEZONE_MEXICO = "CST6CDT,M4.1.0,M10.5.0"; // Mexico timezone

NTPTimeSync::NTPTimeSync() {
    isInitialized = false;
    isTimeSynced = false;
    lastSyncAttempt = 0;
    lastSyncSuccess = 0;
}

bool NTPTimeSync::begin() {
    if (isInitialized) {
        return true;
    }
    
    UnifiedLogger::getInstance().info("Inicializando sincronización NTP...", "NTP");
    
    // Configure timezone
    configureTimezone();
    
    // If WiFi is connected, try initial sync
    if (WiFi.status() == WL_CONNECTED) {
        if (attemptSync()) {
            UnifiedLogger::getInstance().info("✅ Sincronización NTP inicial exitosa", "NTP");
            isTimeSynced = true;
        } else {
            UnifiedLogger::getInstance().warning("⚠️ Falló sincronización NTP inicial, reintentando...", "NTP");
            startLocalClock();
        }
    } else {
        UnifiedLogger::getInstance().info("📡 WiFi no conectado, usando reloj local estimado", "NTP");
        startLocalClock();
    }
    
    isInitialized = true;
    return true;
}

void NTPTimeSync::loop() {
    if (!isInitialized) return;
    
    unsigned long now = millis();
    
    // Check if we need to sync
    if (WiFi.status() == WL_CONNECTED && needsSync()) {
        if (now - lastSyncAttempt >= SYNC_RETRY_INTERVAL) {
            lastSyncAttempt = now;
            
            if (attemptSync()) {
                UnifiedLogger::getInstance().info("✅ Resincronización NTP exitosa", "NTP");
                isTimeSynced = true;
                lastSyncSuccess = now;
                
                if (onSyncCallback) {
                    onSyncCallback(true);
                }
            } else {
                UnifiedLogger::getInstance().warning("⚠️ Falló resincronización NTP", "NTP");
                
                if (onSyncCallback) {
                    onSyncCallback(false);
                }
            }
        }
    }
}

bool NTPTimeSync::attemptSync() {
    UnifiedLogger::getInstance().info("🕐 Intentando sincronización NTP...", "NTP");
    
    // Configure NTP
    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
    
    // Wait for time to be set (up to 10 seconds)
    int timeout = 100; // 10 seconds (100 * 100ms)
    while (time(nullptr) < 100000 && timeout > 0) {
        delay(100);
        timeout--;
    }
    
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* timeinfo = localtime(&now);
        UnifiedLogger::getInstance().info("✅ Tiempo sincronizado: " + String(asctime(timeinfo)), "NTP");
        return true;
    }
    
    UnifiedLogger::getInstance().error("❌ Timeout en sincronización NTP", "NTP");
    return false;
}

void NTPTimeSync::configureTimezone() {
    // Set timezone for Mexico
    setenv("TZ", TIMEZONE_MEXICO, 1);
    tzset();
    UnifiedLogger::getInstance().info("🌍 Zona horaria configurada: México (CST/CDT)", "NTP");
}

bool NTPTimeSync::syncNow() {
    if (WiFi.status() != WL_CONNECTED) {
        UnifiedLogger::getInstance().warning("⚠️ No se puede sincronizar: WiFi desconectado", "NTP");
        return false;
    }
    
    lastSyncAttempt = millis();
    bool success = attemptSync();
    
    if (success) {
        isTimeSynced = true;
        lastSyncSuccess = millis();
    }
    
    return success;
}

uint32_t NTPTimeSync::getCurrentTimestamp() const {
    if (isTimeSynced) {
        return (uint32_t)time(nullptr);
    } else {
        // Use estimated time based on millis()
        return getEstimatedTimestamp();
    }
}

String NTPTimeSync::getCurrentTimeString() const {
    uint32_t timestamp = getCurrentTimestamp();
    return getFormattedTime(timestamp);
}

String NTPTimeSync::getFormattedTime(uint32_t timestamp) const {
    time_t rawTime = (time_t)timestamp;
    struct tm* timeinfo = localtime(&rawTime);
    
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return String(buffer);
}

NTPTimeSync::TimeInfo NTPTimeSync::getTimeInfo() const {
    TimeInfo info;
    info.timestamp = getCurrentTimestamp();
    info.masterTimestamp = info.timestamp; // Same for master
    info.timezoneOffset = -6; // Mexico CST
    info.isValid = isTimeSynced;
    return info;
}

bool NTPTimeSync::setTimeFromMaster(const TimeInfo& timeInfo) {
    if (!timeInfo.isValid) {
        return false;
    }
    
    // Set system time from master
    struct timeval tv;
    tv.tv_sec = timeInfo.masterTimestamp;
    tv.tv_usec = 0;
    
    if (settimeofday(&tv, NULL) == 0) {
        isTimeSynced = true;
        lastSyncSuccess = millis();
        
        UnifiedLogger::getInstance().info("🔄 Tiempo sincronizado desde Master ESP-NOW", "NTP");
        UnifiedLogger::getInstance().info("⏰ Nueva hora: " + getFormattedTime(timeInfo.masterTimestamp), "NTP");
        
        return true;
    }
    
    return false;
}

String NTPTimeSync::getStatusString() const {
    String status = "NTP Status: ";
    
    if (isTimeSynced) {
        status += "✅ Sincronizado";
        if (lastSyncSuccess > 0) {
            unsigned long timeSinceSync = (millis() - lastSyncSuccess) / 1000;
            status += " (hace " + String(timeSinceSync) + "s)";
        }
    } else {
        if (WiFi.status() == WL_CONNECTED) {
            status += "⚠️ Sin sincronizar";
        } else {
            status += "📡 Reloj local (sin WiFi)";
        }
    }
    
    return status;
}

bool NTPTimeSync::needsSync() const {
    if (!isTimeSynced) return true;
    
    unsigned long timeSinceLastSync = millis() - lastSyncSuccess;
    return timeSinceLastSync >= SYNC_INTERVAL;
}

void NTPTimeSync::startLocalClock() {
    // Set a reasonable starting time (2025-01-01 00:00:00)
    struct timeval tv;
    tv.tv_sec = 1735689600; // 2025-01-01 00:00:00 UTC
    tv.tv_usec = 0;
    
    settimeofday(&tv, NULL);
    
    UnifiedLogger::getInstance().info("🕐 Reloj local iniciado con tiempo estimado", "NTP");
}

uint32_t NTPTimeSync::getEstimatedTimestamp() const {
    // Get current time (either synced or estimated)
    return (uint32_t)time(nullptr);
}

bool NTPTimeSync::isUsingEstimatedTime() const {
    return !isTimeSynced;
}

// Time utilities implementations - non-duplicated ones only
String NTPTimeSync::getCurrentDateString() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "__/__/____";  // Evitar trigrafos
    }
    
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &timeinfo);
    return String(buffer);
}

String NTPTimeSync::getCurrentDateTimeString() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "__/__ __:__";  // Evitar trigrafos
    }
    
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    return String(buffer);
}

String NTPTimeSync::getFormattedTime(const char* format) const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "N/A";
    }
    
    char buffer[64];
    strftime(buffer, sizeof(buffer), format, &timeinfo);
    return String(buffer);
}

// Compact display formats for OLED
String NTPTimeSync::getCompactTime() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "__:__";  // Evitar trigrafos
    }
    
    char buffer[8];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    return String(buffer);
}

String NTPTimeSync::getCompactDate() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "__/__";  // Evitar trigrafos
    }
    
    char buffer[8];
    strftime(buffer, sizeof(buffer), "%d/%m", &timeinfo);
    return String(buffer);
}

String NTPTimeSync::getCompactDateTime() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "__/__ __:__";  // Evitar trigrafos
    }
    
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d/%m %H:%M", &timeinfo);
    return String(buffer);
}