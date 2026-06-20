/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "secret_logic.h"
#include <sdkconfig.h>
#include <stdio.h>
#include <string>
#include <esp_log.h>
#include <ArduinoJson.h>
#include <dirent.h>
#include "ssid_manager.h"

namespace secret_logic {

__attribute__((weak)) std::string get_server_url()
{
    static std::string cached_url = "";
    if (!cached_url.empty()) {
        return cached_url;
    }

    // Debug: list all files in /sdcard
    DIR *dir = opendir("/sdcard");
    if (dir != nullptr) {
        ESP_LOGI("secret_logic", "Listing files in /sdcard:");
        struct dirent *ent;
        while ((ent = readdir(dir)) != nullptr) {
            ESP_LOGI("secret_logic", "  - '%s' (type: %d)", ent->d_name, ent->d_type);
        }
        closedir(dir);
    } else {
        ESP_LOGW("secret_logic", "Failed to open directory /sdcard");
    }

    FILE* f = fopen("/sdcard/config.json", "r");
    if (f != nullptr) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (size > 0 && size < 4096) {
            char* buf = (char*)malloc(size + 1);
            if (buf != nullptr) {
                size_t read_bytes = fread(buf, 1, size, f);
                buf[read_bytes] = '\0';
                
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, buf);
                if (!error) {
                    if (doc.containsKey("wifi_ssid")) {
                        std::string wifi_ssid = doc["wifi_ssid"].as<std::string>();
                        std::string wifi_password = doc.containsKey("wifi_password") ? doc["wifi_password"].as<std::string>() : "";
                        SsidManager::GetInstance().AddSsid(wifi_ssid, wifi_password);
                        ESP_LOGI("secret_logic", "Registered WiFi SSID from SD card with SsidManager: %s", wifi_ssid.c_str());
                    }
                    if (doc.containsKey("server_url")) {
                        cached_url = doc["server_url"].as<std::string>();
                        ESP_LOGI("secret_logic", "Loaded server_url from SD card: %s", cached_url.c_str());
                        free(buf);
                        fclose(f);
                        return cached_url;
                    }
                } else {
                    ESP_LOGW("secret_logic", "Failed to parse /sdcard/config.json: %s", error.c_str());
                }
                free(buf);
            }
        }
        fclose(f);
    } else {
        ESP_LOGI("secret_logic", "/sdcard/config.json not found, using default URL");
    }

#ifdef CONFIG_STACKCHAN_SERVER_URL
    return CONFIG_STACKCHAN_SERVER_URL;
#else
    return "http://localhost:3000";
#endif
}

__attribute__((weak)) std::string get_token()
{
    static std::string cached_token = "";
    if (!cached_token.empty()) {
        return cached_token;
    }

    FILE* f = fopen("/sdcard/config.json", "r");
    if (f != nullptr) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (size > 0 && size < 4096) {
            char* buf = (char*)malloc(size + 1);
            if (buf != nullptr) {
                size_t read_bytes = fread(buf, 1, size, f);
                buf[read_bytes] = '\0';
                
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, buf);
                if (!error) {
                    if (doc.containsKey("token")) {
                        cached_token = doc["token"].as<std::string>();
                        ESP_LOGI("secret_logic", "Loaded token from SD card: %s", cached_token.c_str());
                        free(buf);
                        fclose(f);
                        return cached_token;
                    }
                }
                free(buf);
            }
        }
        fclose(f);
    }

    cached_token = "stackchan";
    ESP_LOGI("secret_logic", "No token found in SD card, using default: %s", cached_token.c_str());
    return cached_token;
}

__attribute__((weak)) std::string generate_auth_token()
{
    return "hi-stack-chan";
}

__attribute__((weak)) std::string generate_handshake_token(std::string_view data)
{
    return "hi-stack-chan";
}

}  // namespace secret_logic
