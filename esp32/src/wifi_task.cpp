#include "wifi_task.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include <cstring>
#include <socket.h>


static constexpr char SSID[] = "";
static constexpr char PASSWORD[] = "";

namespace task {

namespace wifi_task {
static WifiTask* handle;

static const wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

static void init() {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    esp_wifi_init(&config);

    wifi_config_t wifi_config{};
    
    std::strncpy(
        reinterpret_cast<char*>(wifi_config.sta.ssid),
        SSID,
        sizeof(wifi_config.sta.ssid)
    );

    std::strncpy(
        reinterpret_cast<char*>(wifi_config.sta.password),
        PASSWORD,
        sizeof(wifi_config.sta.password)
    );

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
}


static void event_callback(
    void*,
    esp_event_base_t event_base,
    int32_t event_id,
    void*
)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        handle->notify(static_cast<uint32_t>(WifiTask::Event::DISCONNECTED));
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        handle->notify(static_cast<uint32_t>(WifiTask::Event::CONNECTED));
    }
}

}


WifiTask* WifiTask::handle() {
    return wifi_task::handle;
}

bool WifiTask::connected() {
    return connected_;
}

void WifiTask::start() {
    wifi_task::handle = this;

    wifi_task::init();
    esp_wifi_start();
    esp_wifi_connect();

    esp_event_handler_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        &wifi_task::event_callback,
        nullptr
    );

    esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_task::event_callback,
        nullptr
    );

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    while (true) {
        // Check for a connection event
        uint32_t event = wait(0);

        if (event & static_cast<uint32_t>(Event::CONNECTED)) {
            connected_ = true;
        } else if (event & static_cast<uint32_t>(Event::DISCONNECTED)) {
            connected_ = false;
            esp_wifi_connect();
        }


        if (connected_) {
            sockaddr_in source{};
            socklen_t source_len = sizeof(source);

            int len = recvfrom(
                sock,
                recv_buffer_,
                sizeof(recv_buffer_),
                0,
                reinterpret_cast<sockaddr*>(&source),
                &source_len
            );
            if (len > 0) {
                receivePacket(len);
            }
        }
    }
}

void WifiTask::receivePacket(size_t len) {
    ESP_LOGI("WifiTask", "Received %d bytes: %.*s", len, len, recv_buffer_);
}
};