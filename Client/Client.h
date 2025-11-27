//
// Created by pedro-souza on 24/11/2025.
//
#pragma once
#include "../Network/Platforms.h"
#include <cstdint>

#include "InputData.h"

enum class ClientState {
    CLIENT_DOWN,
    CLIENT_SET,
    CLIENT_READY,
    CLIENT_RUNNING,
};

class Client {
public:
    Client();
    ~Client() = default;

    void Initialize();
    bool AddServerAddr(const char *serverIp);
    bool Connect();
    void SendCommandsToServer() const;
    void Shutdown();

    void SetNonce(const uint32_t nonce) { mClientNonce = nonce; }
    void IncreasePacketSequence() { mCurrentPacketSequence++; }

    [[nodiscard]] int GetSocket() const { return mSocket;}
    [[nodiscard]] sockaddr_in GetServerAddress() const { return mServerAddrV4; }
    [[nodiscard]] uint32_t GetClientNonce() const { return mClientNonce; }
    [[nodiscard]] uint16_t GetCurrentPacketSequence() const { return mCurrentPacketSequence; }

    static constexpr int MAX_CONNECTION_ATTEMPTS = 6;
    static constexpr int CONNECTION_RECEIVING_TIMEOUT_IN_MS = 2000;

    // Inputs Control
    [[nodiscard]] InputData *GetInputData() const { return mInputData; }
private:
    ClientState mState;
    SocketType mSocket;
    sockaddr_in mServerAddrV4;

    // Connection control
    uint16_t mCurrentPacketSequence;
    uint32_t mClientNonce;

    // // Inputs Control
    InputData *mInputData;
};