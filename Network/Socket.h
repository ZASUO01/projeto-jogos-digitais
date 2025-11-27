//
// Created by pedro-souza on 23/11/2025.
//
#pragma once
#include "Platforms.h"
#include "Packet.h"

namespace  SocketUtils {
    SocketType createSocketV4();
    void bindSocketToAnyV4(SocketType sock);
    bool socketReadyToReceive(SocketType sock, int ms);
    bool sendPacketToV4(SocketType sock, Packet *pk, size_t pkSize, sockaddr_in* addr4);
    bool receivePacketFromV4(SocketType sock, Packet *pk, sockaddr_in* addr4);
};
