//
// Created by pedro-souza on 23/11/2025.
//

#include "Socket.h"
#include "Logger.h"
#include "Addresses.h"
#include "Defs.h"
#include <cstring>

SocketType SocketUtils::createSocketV4() {
    const SocketType sock = create_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        Logger::sysLogExit("create socket");
    }

    ignore_not_reachable_error(sock);
    return sock;
}

void SocketUtils::bindSocketToAnyV4(const SocketType sock) {
    sockaddr_in addr{};
    Addresses::initAddrAnyV4(&addr, APP_PORT);

    if (socket_bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        close_socket(sock);
        Logger::sysLogExit("bind socket");
    }
}

bool SocketUtils::socketReadyToReceive(const SocketType sock, const int ms) {
    POLL_FD_TYPE fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    if (const int result = socket_poll(fds, 1, ms); result > 0 && (fds[0].revents & POLLIN)) {
        return true;
    }
    
    return false;
}

bool SocketUtils::sendPacketToV4(const SocketType sock, Packet *pk, const size_t pkSize, sockaddr_in * addr4) {
    constexpr socklen_t addr_size = sizeof(sockaddr_in);

    if (const ssize_t bytes_sent = socket_sendto(sock, pk, pkSize, 0, reinterpret_cast<sockaddr *>(addr4), addr_size);
        bytes_sent < 0 || static_cast<size_t>(bytes_sent) != pkSize) {
        return false;
        }
    return true;
}

bool SocketUtils::receivePacketFromV4(const SocketType sock, Packet *pk, sockaddr_in * addr4) {
    constexpr size_t pkSize = Packet::PACKET_HEADER_BYTES + Packet::MAX_PACKET_DATA_BYTES;
    std::memset(pk, 0, pkSize);

    socklen_t addrSize = sizeof(sockaddr_in);

    if (const ssize_t bytes_received = socket_recvfrom(sock, pk, pkSize, 0, reinterpret_cast<sockaddr *>(addr4), &addrSize); bytes_received <= 0) {
        return false;
    }
    return true;
}
