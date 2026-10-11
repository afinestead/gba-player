#include "wifi_task.h"
#include "serial_task.h"
#include "libs/include/helpers.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include <cstring>
#include <algorithm>


namespace task {

namespace wifi_task {
static WifiTask* handle;
}

static void recordToStr(const wifi_ap_record_t& record, char* buffer, size_t buffer_size) {
    char* buf_ptr = buffer;
    uint8_t* ssid_ptr = const_cast<uint8_t*>(record.ssid);
    for (uint8_t ii = 0; ii < buffer_size - 7 && *ssid_ptr; ++ii) {
        *buf_ptr++ = *ssid_ptr++;
    }
    std::fill(buf_ptr, buffer + buffer_size, ' ');
    if (*ssid_ptr) {
        std::fill(buf_ptr, buffer + buffer_size - 4, '.');
    }

    const uint8_t rssi_strength = (
        record.rssi > -50 ? 3 :
        record.rssi > -70 ? 2 :
                            1
    );
    for (uint8_t ii = buffer_size; ii >= buffer_size - rssi_strength; ii--) {
        buffer[ii] = '*';
    }   
}

static void event_callback(
    void*,
    esp_event_base_t event_base,
    int32_t event_id,
    void*
) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_task::handle->notify(static_cast<uint32_t>(WifiTask::Event::DISCONNECTED));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_task::handle->notify(static_cast<uint32_t>(WifiTask::Event::CONNECTED));
    }
}

WifiTask* WifiTask::handle() {
    return wifi_task::handle;
}

bool WifiTask::connected() {
    return state_ == State::CONNECTED;
}

void WifiTask::scanNetworks() {
    notify(static_cast<uint32_t>(Event::START_SCAN));
}

void WifiTask::connect() {
    notify(static_cast<uint32_t>(Event::CONNECT));
}

void WifiTask::disconnect() {
    notify(static_cast<uint32_t>(Event::DISCONNECT));
}

void WifiTask::performScan() {
    uint16_t num_networks = 0;
    esp_wifi_scan_start(nullptr, true);
    esp_wifi_scan_get_ap_num(&num_networks);

    num_networks = std::min(num_networks, static_cast<uint16_t>(MAX_AP_RECORDS));

    esp_wifi_scan_get_ap_records(&num_networks, ap_records_);

    std::sort(
        ap_records_,
        ap_records_ + num_networks,
        [](const wifi_ap_record_t& left, const wifi_ap_record_t& right) {
            return left.rssi > right.rssi;
        }
    );

    for (int i = 0; i < num_networks; ++i) {
        SerialTask::Message msg;
        recordToStr(ap_records_[i], msg.s, sizeof(msg.s));
        SerialTask::handle()->enqueueMessage(msg);
    }

    if (!auto_connect_) {
        return;
    }

    for (const auto& creds : known_credentials_) {
        for (const auto& record : ap_records_) {
            if (!std::strncmp(creds.ssid, reinterpret_cast<const char*>(record.ssid), sizeof(record.ssid))) {
                std::copy_n(creds.ssid, sizeof(creds.ssid), config_.sta.ssid);
                std::copy_n(creds.password, sizeof(creds.password), config_.sta.password);
                notify(static_cast<uint32_t>(Event::CONNECT));
                return;
            }
        }
    }
}

void WifiTask::start() {
    wifi_task::handle = this;
    nvs_driver_.init();

    nvs_driver_.load(
        NVS_CREDS_NS,
        "creds0",
        reinterpret_cast<uint8_t*>(known_credentials_),
        sizeof(Credentials) * MAX_STORED_CREDENTIALS
    );

    const wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    esp_wifi_init(&init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);

    config_.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;    
    esp_wifi_set_config(WIFI_IF_STA, &config_);

    esp_wifi_start();

    esp_event_handler_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        &event_callback,
        nullptr
    );

    esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_callback,
        nullptr
    );

    notify(static_cast<uint32_t>(Event::START_SCAN));

    State last_logged_state = state_;
    while (true) {
        uint32_t event = wait(10000);

        if ((event & static_cast<uint32_t>(Event::START_SCAN)) && state_ == State::DISCONNECTED) {
            state_ = State::SCANNING;
            performScan();
        }

        if ((event & static_cast<uint32_t>(Event::CONNECT)) && state_ == State::SCANNING) {
            esp_wifi_set_config(WIFI_IF_STA, &config_);
            esp_wifi_connect();
        }

        if ((event & static_cast<uint32_t>(Event::CONNECTED)) && state_ == State::SCANNING) {
            state_ = State::CONNECTED;
        }

        if (event & static_cast<uint32_t>(Event::DISCONNECT)) {
            if (state_ == State::CONNECTED) {
                auto_connect_ = false;
                esp_wifi_disconnect();
            }
        }

        if ((event & static_cast<uint32_t>(Event::DISCONNECTED)) && state_ != State::DISCONNECTED) {
            state_ = State::DISCONNECTED;
        }

        if (state_ != last_logged_state) {
            switch (state_) {
                case State::DISCONNECTED:
                    SerialTask::handle()->enqueueMessage({"DISCONNECTED"});
                    break;
                case State::SCANNING:
                    SerialTask::handle()->enqueueMessage({"SCANNING"});
                    break;
                case State::CONNECTED:
                    SerialTask::handle()->enqueueMessage({"CONNECTED"});
                    break;
                default:
                    SerialTask::handle()->enqueueMessage({"UNKNOWN"});
                    break;
            }
            last_logged_state = state_;
        }
    }
}

}