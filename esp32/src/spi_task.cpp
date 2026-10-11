#include "spi_task.h"

#include "network_task.h"

namespace task {

namespace spi_task {
static SpiTask* handle;
}

SpiTask* SpiTask::handle() {
	return spi_task::handle;
}

void SpiTask::start() {
	spi_task::handle = this;

	while (true) {
		auto frame = NetworkTask::buffers_.acquireReady();
		if (frame) {
			NetworkTask::buffers_.publishFree(frame->index);
		}
	}
}

}
