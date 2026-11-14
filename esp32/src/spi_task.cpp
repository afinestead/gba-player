#include "spi_task.h"
#include "network_task.h"

namespace task {

namespace spi_task {
static SpiTask* handle;
}

static void IRAM_ATTR transmit_complete_callback(spi_transaction_t* trans) {
    SpiTask::handle()->notifyFromISR(
        static_cast<uint32_t>(SpiTask::Event::TRANSMIT_COMPLETE)
    );
}

SpiTask* SpiTask::handle() {
	return spi_task::handle;
}

void SpiTask::start() {
	spi_task::handle = this;

    spi_bus_config_t bus_config;
    bus_config.mosi_io_num = 23;
    bus_config.miso_io_num = 19;
    bus_config.sclk_io_num = 18;
    bus_config.quadwp_io_num = -1;
    bus_config.quadhd_io_num = -1;
    bus_config.max_transfer_sz = NetworkTask::buffers_.size();

    spi_device_interface_config_t device_config;
    device_config.command_bits = 0;
    device_config.address_bits = 0;
    device_config.dummy_bits = 0;
    device_config.mode = 0;
    device_config.spics_io_num = 5;
    device_config.queue_size = 1;
    device_config.post_cb = transmit_complete_callback;

    spi_driver_.init(bus_config, device_config);

	while (true) {
        auto frame = NetworkTask::buffers_.acquireReady();
        if (!frame) {
            continue;
        }
        spi_driver_.sendBlocking(
            reinterpret_cast<const uint8_t*>(frame->header()),
            sizeof(ProtocolHeader) + frame->header()->length
        );
        NetworkTask::buffers_.publishFree(frame->index);
	}
}


}
