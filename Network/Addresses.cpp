//
// Created by pedro-souza on 24/11/2025.
//
#include "Addresses.h"

void Addresses::initAddrAnyV4(sockaddr_in *addr4, const unsigned int port) {
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY;
    addr4->sin_port = htons(port);
}

bool Addresses::parseAddrV4(sockaddr_in *addr, const char *addrStr, const uint16_t port) {
    in_addr addr_v4{};
    if (inet_pton(AF_INET, addrStr, &addr_v4) != 1) {
        return false;
    }

    addr->sin_addr = addr_v4;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    return true;
}