//
// Created by pedro-souza on 24/11/2025.
//
#pragma once
#include "../Network/Platforms.h"
#include "DataObjects.h"
#include "../Source/Game.h"
#include <vector>
#include <SDL.h>

enum class ClientState {
    CLIENT_DOWN,
    CLIENT_SET,
    CLIENT_READY,
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED,
};

class Client {
public:
    explicit Client(Game *game);
    ~Client() = default;

    void Initialize();
    bool AddServerAddr(const char *serverIp);
    bool Connect();


    bool Disconnect();
    void Shutdown();

    void SetNonce(const uint32_t nonce) { mClientNonce = nonce; }
    void IncreasePacketSequence() { mCurrentPacketSequence++; }

    [[nodiscard]] SocketType GetSocket() const { return mSocket;}
    [[nodiscard]] sockaddr_in GetServerAddress() const { return mServerAddrV4; }
    [[nodiscard]] uint32_t GetClientNonce() const { return mClientNonce; }
    [[nodiscard]] uint16_t GetCurrentPacketSequence() const { return mCurrentPacketSequence; }

    static constexpr int MAX_CONNECTION_ATTEMPTS = 10;
    static constexpr int CONNECTION_RECEIVING_TIMEOUT_IN_MS = 2000;

    // Inputs Control
    void AddInput(const Uint8 *keyState);
    [[nodiscard]] const std::vector<Command>& GetCommands() const { return mCommands; }
    void SendCommandsToServer() const;

    // State control
    void ReceiveStateFromServer();
    void SetLastReceivedInputSequence(const uint32_t inputSequence) { mLastReceivedInputSequence = inputSequence; }

    void SetRawState(const RawState& state) { mRawState = state; }
    void SetOtherState(const OtherState& state) { mOtherState = state; }
private:
    ClientState mState;
    SocketType mSocket;
    sockaddr_in mServerAddrV4;

    // Connection control
    uint16_t mCurrentPacketSequence;
    uint32_t mClientNonce;
    bool mConnecting;
    bool mDisconnecting;

    // Inputs Control
    std::vector<Command> mCommands;
    static uint32_t mCurrentCommandSequence;
    void CleanConfirmedCommands(uint32_t confirmedSequence);

    // State control
    RawState mRawState;
    OtherState mOtherState;
    uint32_t mLastReceivedInputSequence;
    uint32_t mLasRemovedInputSequence;
    void ReprocessLocalState() const;

    // Game owner
    Game *mGame;
};