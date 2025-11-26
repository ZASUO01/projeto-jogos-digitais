//
// Created by pedro-souza on 24/11/2025.
//
#pragma once
#include "../Network/Platforms.h"
#include <cstdint>

enum class ClientState {
    CLIENT_DOWN,
    CLIENT_SET,
    CLIENT_READY,
};

class Client {
public:
    Client();
    ~Client() = default;

    void Initialize();
    bool AddServerAddr(const char *serverIp);
    bool Connect();

    void SetNonce(const uint32_t nonce) { mClientNonce = nonce; }
    void IncreasePacketSequence() { mCurrentPacketSequence++; }

    [[nodiscard]] int GetSocket() const { return mSocket;}
    [[nodiscard]] sockaddr_in GetServerAddress() const { return mServerAddrV4; }
    [[nodiscard]] uint32_t GetClientNonce() const { return mClientNonce; }
    [[nodiscard]] uint16_t GetCurrentPacketSequence() const { return mCurrentPacketSequence; }

    static constexpr int MAX_CONNECTION_ATTEMPTS = 6;
    static constexpr int CONNECTION_RECEIVING_TIMEOUT_IN_MS = 2000;
private:

    ClientState mState;
    int mSocket;
    sockaddr_in mServerAddrV4;

    // Connection control
    uint16_t mCurrentPacketSequence;
    uint32_t mClientNonce;
};