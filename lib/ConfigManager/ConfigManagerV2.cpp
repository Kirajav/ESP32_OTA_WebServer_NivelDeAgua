#include "ConfigManagerV2.h"

const char* ConfigManagerV2::CONFIG_FILE = "/config.json";

ConfigManagerV2::ConfigManagerV2() {
    // Domain objects are initialized with their defaults
}

bool ConfigManagerV2::begin() {
    if (!mountSPIFFS()) {
        return false;
    }
    
    // Try to load existing configuration
    if (!loadFromFile()) {
        // If loading fails, use defaults and save them
        saveToFile();
    }
    
    return true;
}

bool ConfigManagerV2::mountSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Error: Failed to mount SPIFFS");
        return false;
    }
    return true;
}

bool ConfigManagerV2::loadFromFile() {
    if (!SPIFFS.exists(CONFIG_FILE)) {
        Serial.println("Config file doesn't exist, using defaults");
        return false;
    }
    
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Error: Failed to open config file for reading");
        return false;
    }
    
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("Error: Failed to parse config JSON: %s\n", error.c_str());
        return false;
    }
    
    JsonObject root = doc.as<JsonObject>();
    
    // Load each domain configuration
    bool network_ok = true, sensor_ok = true, system_ok = true;
    
    if (root.containsKey("network")) {
        network_ok = network_config.loadFromJson(root["network"]);
    } else {
        // Backward compatibility: load from root level
        network_ok = network_config.loadFromJson(root);
    }
    
    if (root.containsKey("sensor")) {
        sensor_ok = sensor_config.loadFromJson(root["sensor"]);
    } else {
        // Backward compatibility: load from root level
        sensor_ok = sensor_config.loadFromJson(root);
    }
    
    if (root.containsKey("system")) {
        system_ok = system_config.loadFromJson(root["system"]);
    } else {
        // Backward compatibility: load from root level
        system_ok = system_config.loadFromJson(root);
    }
    
    // Load ESP-NOW configuration
    bool espnow_ok = true;
    if (root.containsKey("espnow")) {
        espnow_config.fromJson(root["espnow"]);
    } else {
        // Initialize with defaults if not present
        espnow_config.initialize();
    }
    
    if (!network_ok || !sensor_ok || !system_ok || !espnow_ok) {
        Serial.println("Warning: Some configuration domains failed validation, using defaults");
    }
    
    return true;
}

bool ConfigManagerV2::saveToFile() {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    JsonObject root = doc.to<JsonObject>();
    
    // Save each domain to its own section
    JsonObject network_obj = root.createNestedObject("network");
    network_config.saveToJson(network_obj);
    
    JsonObject sensor_obj = root.createNestedObject("sensor");
    sensor_config.saveToJson(sensor_obj);
    
    JsonObject system_obj = root.createNestedObject("system");
    system_config.saveToJson(system_obj);
    
    JsonObject espnow_obj = root.createNestedObject("espnow");
    espnow_config.toJson(espnow_obj);
    
    // Also save to root level for backward compatibility
    network_config.saveToJson(root);
    sensor_config.saveToJson(root);
    system_config.saveToJson(root);
    
    File file = SPIFFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Error: Failed to open config file for writing");
        return false;
    }
    
    size_t bytes_written = serializeJson(doc, file);
    file.close();
    
    if (bytes_written == 0) {
        Serial.println("Error: Failed to write config file");
        return false;
    }
    
    Serial.printf("Configuration saved (%d bytes)\n", bytes_written);
    return true;
}

bool ConfigManagerV2::save() {
    return saveToFile();
}

bool ConfigManagerV2::load() {
    return loadFromFile();
}

void ConfigManagerV2::reset() {
    network_config.setDefaults();
    sensor_config.setDefaults();
    system_config.setDefaults();
    saveToFile();
    Serial.println("Configuration reset to defaults");
}

bool ConfigManagerV2::isValid() const {
    return network_config.isValid() && 
           sensor_config.isValid() && 
           system_config.isValid();
}

bool ConfigManagerV2::exportToJson(String& json_string) const {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    JsonObject root = doc.to<JsonObject>();
    
    // Export each domain
    JsonObject network_obj = root.createNestedObject("network");
    network_config.saveToJson(network_obj);
    
    JsonObject sensor_obj = root.createNestedObject("sensor");
    sensor_config.saveToJson(sensor_obj);
    
    JsonObject system_obj = root.createNestedObject("system");
    system_config.saveToJson(system_obj);
    
    // Also export to root level for compatibility
    network_config.saveToJson(root);
    sensor_config.saveToJson(root);
    system_config.saveToJson(root);
    
    json_string = "";
    serializeJsonPretty(doc, json_string);
    
    return !json_string.isEmpty();
}

bool ConfigManagerV2::importFromJson(const String& json_string) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, json_string);
    
    if (error) {
        Serial.printf("Error: Failed to parse import JSON: %s\n", error.c_str());
        return false;
    }
    
    JsonObject root = doc.as<JsonObject>();
    
    // Load each domain
    bool success = true;
    
    if (root.containsKey("network")) {
        success &= network_config.loadFromJson(root["network"]);
    } else {
        success &= network_config.loadFromJson(root);
    }
    
    if (root.containsKey("sensor")) {
        success &= sensor_config.loadFromJson(root["sensor"]);
    } else {
        success &= sensor_config.loadFromJson(root);
    }
    
    if (root.containsKey("system")) {
        success &= system_config.loadFromJson(root["system"]);
    } else {
        success &= system_config.loadFromJson(root);
    }
    
    if (success) {
        saveToFile();
        Serial.println("Configuration imported successfully");
    } else {
        Serial.println("Error: Configuration import failed validation");
    }
    
    return success;
}
