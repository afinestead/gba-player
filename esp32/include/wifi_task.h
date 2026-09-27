#pragma once

#include "task.h"
#include "esp_wifi.h"
#include "nvs_driver.h"

namespace task {

using WifiTaskBase = Task<4096>;

class WifiTask : public WifiTaskBase {
public:
    enum class State : uint8_t {
        DISCONNECTED,
        SCANNING,
        CONNECTED,
    };

    enum class Event : uint32_t {
        CONNECTED = 1 << 0,
        DISCONNECTED = 1 << 1,
        START_SCAN = 1 << 2,
        CONNECT = 1 << 3,
        DISCONNECT = 1 << 4,
    };

    struct Credentials {
        char ssid[33];
        char password[65];
    };

    WifiTask() :
        WifiTaskBase("WifiTask", Priority::WIFI)
    {}

    static WifiTask* handle();

    bool connected();
    
    void scanNetworks();
    void connect();
    void disconnect();

protected:
    void start() override;
private:
    static constexpr uint16_t MAX_AP_RECORDS = 20;
    static constexpr uint8_t MAX_STORED_CREDENTIALS = 5;

    static constexpr char NVS_PART[] = "wifi";
    static constexpr char NVS_CREDS_NS[] = "credentials";

    State state_ = State::DISCONNECTED;

    bool auto_connect_ = true;

    wifi_ap_record_t ap_records_[MAX_AP_RECORDS];

    void performScan();


    platform::NvsDriver nvs_driver_ = platform::NvsDriver(NVS_PART);

    wifi_config_t config_;
    Credentials known_credentials_[MAX_STORED_CREDENTIALS];
};

} // namespace task