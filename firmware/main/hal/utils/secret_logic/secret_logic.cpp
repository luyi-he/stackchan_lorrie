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

namespace secret_logic {

__attribute__((weak)) std::string get_server_url()
{
    static std::string cached_url = "";
    if (!cached_url.empty()) {
        return cached_url;
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

__attribute__((weak)) std::string generate_auth_token()
{
    return "hi-stack-chan";
}

__attribute__((weak)) std::string generate_handshake_token(std::string_view data)
{
    return "hi-stack-chan";
}

}  // namespace secret_logic
