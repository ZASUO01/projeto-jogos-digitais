//
// Created by pedro-souza on 24/11/2025.
//
#include "Client.h"
#include "../Network/Socket.h"
#include "../Network/Addresses.h"
#include "../Network/Defs.h"
#include "SDL.h"

Client::Client()
:mState(ClientState::CLIENT_DOWN)
,socket(-1)
,serverAddrV4{}
{}

void Client::Initialize(const char *serverIp) {
    if (mState != ClientState::CLIENT_DOWN) {
        return;
    }
    socket = SocketUtils::createSocketV4();

    if (!Addresses::parseAddrV4(&serverAddrV4, serverIp, APP_PORT)) {
        return;
    }

    mState = ClientState::CLIENT_SET;

    SDL_Log("Client initialized");
}

bool Client::Connect() {
    if (mState != ClientState::CLIENT_SET) {
        return false;
    }


}

