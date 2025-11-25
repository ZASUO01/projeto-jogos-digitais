//
// Created by pedro-souza on 24/11/2025.
//
#pragma once
#include "../Network/Platforms.h"

enum class ClientState {
    CLIENT_DOWN,
    CLIENT_SET,
};

class Client {
public:
    Client();
    ~Client() = default;

    void Initialize(const char* serverIp);
    bool Connect();

private:
    ClientState mState;
    int socket;
    sockaddr_in serverAddrV4;
};