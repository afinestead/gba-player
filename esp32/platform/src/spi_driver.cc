#include "spi_driver.h"

#include <algorithm>

namespace platform {

SpiDriver::SpiDriver(const spi_host_device_t host) : host_(host) {}

void SpiDriver::init(
    const spi_bus_config_t& config, 
    const spi_device_interface_config_t& device_config
) {
    spi_bus_initialize(host_, &config, SPI_DMA_CH_AUTO);
    spi_bus_add_device(host_, &device_config, &device_);
}

bool SpiDriver::sendBlocking(const uint8_t* data, size_t length, uint32_t timeout_ms) {
    static constexpr size_t CHUNK_SIZE = 32 * 1024;
    for (size_t offset = 0; offset < length; offset += CHUNK_SIZE) {
        size_t chunk_size = std::min(CHUNK_SIZE, length - offset);

        spi_transaction_t trans = {};
        trans.length = chunk_size * 8;
        trans.tx_buffer = data + offset;

        auto res = spi_device_transmit(device_, &trans);
        if (res != ESP_OK) {
            return false;
        }
    }
    return true;
}

}
