#include "driver/spi_master.h"
#include <cstddef>
#include <cstdint>

namespace platform {

class SpiDriver {
public:

    SpiDriver(const spi_host_device_t host);

    void init(
        const spi_bus_config_t& config, 
        const spi_device_interface_config_t& device_config
    );

    spi_device_handle_t device() const;
    bool send(const uint8_t* data, size_t length, uint32_t timeout_ms = 0);
    bool recv(uint8_t* data, size_t length, uint32_t timeout_ms = 0);
private:
    spi_host_device_t host_;
    spi_device_handle_t device_;
    spi_bus_config_t config_;

};

} // namespace platform