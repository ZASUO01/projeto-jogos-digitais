//
// Created by pedro-souza on 24/11/2025.
//
#include "Client.h"

#include "ClientOperations.h"
#include "../Network/Socket.h"
#include "../Network/Addresses.h"
#include "../Network/Defs.h"
#include "SDL.h"
#include "../Network/NetUtils.h"

Client::Client(Game *game)
:mState(ClientState::CLIENT_DOWN)
,mSocket(-1)
,mServerAddrV4{}
,mCurrentPacketSequence(0)
,mClientNonce(0)
,mConnecting(false)
,mDisconnecting(false)
,mInputData(nullptr)
,mGame(game)
{}

void Client::Initialize() {
    if (mState != ClientState::CLIENT_DOWN) {
        return;
    }

    mSocket = SocketUtils::createSocketV4();
    mClientNonce = NetUtils::getNonce();
    mInputData = new InputData();
    mState = ClientState::CLIENT_SET;

    SDL_Log("Client initialized");
}

bool Client::AddServerAddr(const char *serverIp) {
    if (mState != ClientState::CLIENT_SET) {
        return false;
    }

    if (!Addresses::parseAddrV4(&mServerAddrV4, serverIp, APP_PORT)) {
        return false;
    }

    mState = ClientState::CLIENT_READY;

    SDL_Log("Server address added");
    return true;
}


bool Client::Connect() {
    if (mState != ClientState::CLIENT_READY) {
        return false;
    }

    mConnecting = true;

    for (int i = 0; i < MAX_CONNECTION_ATTEMPTS; i++) {
        ClientOperations::sendSinglePacketToServer(this, Packet::SYN_FLAG);

        if (!ClientOperations::receiveSinglePacketFromServer(this, Packet::SYN_ACK_FLAG)) {
            SDL_Log("Response not received, trying again...");
            continue;
        }

        ClientOperations::sendSinglePacketToServer(this, Packet::ACK_FLAG);

        SDL_Log("Client connected");
        mState = ClientState::CLIENT_CONNECTED;
        mConnecting = false;
        return true;
    }

    SDL_Log("Max connection attempts reached");
    mConnecting = false;
    return false;
}

void Client::SendCommandsToServer() const {
    if (mState != ClientState::CLIENT_CONNECTED || mConnecting || mDisconnecting) {
        return;
    }

    ClientOperations::sendDataToServer(this);
}

void Client::ReceiveDataFromServer()  {
    if (mState != ClientState::CLIENT_CONNECTED) {
        return;
    }
    if (!ClientOperations::receiveDataPacketFromServer(this)) {
        return;
    }

    float newX = mLastRawState.posX;
    float newY = mLastRawState.posY;
    mGame->GetShip()->SetPosition(Vector2(newX, newY));
}

bool Client::Disconnect() {
    if (mState != ClientState::CLIENT_CONNECTED) {
        return false;
    }

    mDisconnecting = true;

    for (int i = 0; i < MAX_CONNECTION_ATTEMPTS; i++) {
        ClientOperations::sendSinglePacketToServer(this, Packet::END_FLAG);

        if (!ClientOperations::receiveSinglePacketFromServer(this, Packet::END_ACK_FLAG)) {
            SDL_Log("Response not received, trying again...");
            continue;
        }

        ClientOperations::sendSinglePacketToServer(this, Packet::ACK_FLAG);

        SDL_Log("Client disconnected");
        mState = ClientState::CLIENT_DISCONNECTED;
        mDisconnecting = false;
        return true;
    }

    SDL_Log("Max disconnection attempts reached");
    mDisconnecting = false;
    return false;
}


void Client::Shutdown() {
    if (mState != ClientState::CLIENT_CONNECTED && mState != ClientState::CLIENT_DISCONNECTED) {
        return;
    }

    delete mInputData;
    mInputData = nullptr;
}