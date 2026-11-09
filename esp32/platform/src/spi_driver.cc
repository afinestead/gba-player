#include "spi_driver.h"

namespace platform {

SpiDriver::SpiDriver(const spi_host_device_t host) : host_(host) {}

void SpiDriver::init(
    const spi_bus_config_t& config, 
    const spi_device_interface_config_t& device_config
) {
    spi_bus_initialize(host_, &config, SPI_DMA_CH_AUTO);
    spi_bus_add_device(host_, &device_config, &device_);
}

bool SpiDriver::send(const uint8_t* data, size_t length, uint32_t timeout_ms) {
    spi_transaction_t trans{};
    trans.length = length * 8;
    trans.tx_buffer = data;
    auto res = spi_device_transmit(device_, &trans);
    return res == ESP_OK;
}

}
