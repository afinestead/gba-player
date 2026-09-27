#include "network_task.h"
#include <socket.h>

namespace task {

namespace network_task {
static NetworkTask* handle;
}

NetworkTask* NetworkTask::handle() {
    return network_task::handle;
}

void NetworkTask::start() {
    network_task::handle = this;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    sockaddr_in source{};
    socklen_t source_len = sizeof(source);

    int len = recv(sock, recv_buffer_, sizeof(recv_buffer_), 0);
    if (len > 0) {
        // receivePacket(len);
    }
    
    while (true) {
        delay(1000);
    }
}

} // namespace task
