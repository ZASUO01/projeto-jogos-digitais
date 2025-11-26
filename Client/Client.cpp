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

Client::Client()
:mState(ClientState::CLIENT_DOWN)
,mSocket(-1)
,mServerAddrV4{}
,mCurrentPacketSequence(0)
,mClientNonce(0)
{}

void Client::Initialize() {
    if (mState != ClientState::CLIENT_DOWN) {
        return;
    }

    mSocket = SocketUtils::createSocketV4();
    mClientNonce = NetUtils::getNonce();
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

    for (int i = 0; i < MAX_CONNECTION_ATTEMPTS; i++) {
        ClientOperations::sendSynToServer(this);

        if (!ClientOperations::receiveSynAckFromServer(this)) {
            SDL_Log("Response not received, trying again...");
            continue;
        }

        ClientOperations::sendAckToServer(this);

        SDL_Log("Client connected");
        return true;
    }

    SDL_Log("Max connection attempts reached");
    return false;
}

