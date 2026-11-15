#include "spi_task.h"
#include "network_task.h"

namespace task {

namespace spi_task {
static SpiTask* handle;
}

// static void IRAM_ATTR transmit_complete_callback(spi_transaction_t* trans) {
//     SpiTask::handle()->notifyFromISR(
//         static_cast<uint32_t>(SpiTask::Event::TRANSMIT_COMPLETE)
//     );
// }

SpiTask* SpiTask::handle() {
	return spi_task::handle;
}

libs::logging::Message SpiTask::printStats(uint64_t frames_sent_delta) const {
    libs::logging::Message message{};
    message.appendText("spi stats tx/s:");
    message.appendCount(frames_sent_delta);
    return message;
}

void SpiTask::start() {
	spi_task::handle = this;

    spi_bus_config_t bus_cfg {};
    bus_cfg.mosi_io_num = 11;
    bus_cfg.miso_io_num = 13;
    bus_cfg.sclk_io_num = 12;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = NetworkTask::buffers.size();

    spi_device_interface_config_t dev_cfg {};
    dev_cfg.clock_source = SPI_CLK_SRC_DEFAULT;
    dev_cfg.clock_speed_hz = SPI_MASTER_FREQ_40M;
    dev_cfg.command_bits = 0;
    dev_cfg.address_bits = 0;
    dev_cfg.dummy_bits = 0;
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = 10;
    dev_cfg.queue_size = 1;
    dev_cfg.post_cb = nullptr;

    spi_driver_.init(bus_cfg, dev_cfg);

	while (true) {
        if (auto frame = NetworkTask::buffers.acquireReady()) {
            if (spi_driver_.sendBlocking(
                reinterpret_cast<const uint8_t*>(frame->header()),
                frame->len()
            )) {
                ++frames_sent_;
            }
            NetworkTask::buffers.publishFree(frame->index);
        }
	}
}


}
