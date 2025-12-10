//
// Created by pedro-souza on 24/11/2025.
//
#include "Client.h"
#include <algorithm>
#include "ClientOperations.h"
#include "../Network/Socket.h"
#include "../Network/Addresses.h"
#include "../Network/Defs.h"
#include "SDL.h"
#include "SDLInputParser.h"
#include "../Network/NetUtils.h"
#include "../Source/PathResolver.h"
#include "../Source/UI/Screens/Endgame.h"

uint32_t Client::mCurrentCommandSequence = 0;

Client::Client(Game *game)
    : mState(ClientState::CLIENT_DOWN)
      , mSocket(-1)
      , mServerAddrV4{}
      , mCurrentPacketSequence(0)
      , mClientNonce(0)
      , mConnecting(false)
      , mDisconnecting(false)
      , mLastReceivedInputSequence(0)
      , mLasRemovedInputSequence(0)
      , mGame(game)
{
}

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


void Client::Shutdown() const {
    if (mState == ClientState::CLIENT_DOWN) {
        return;
    }
    close_socket(mSocket);

    SDL_Log("Client shutdown");
}

void Client::AddInput(const Uint8 *keyState) {
    InputData input = SDLInputParser::parse(keyState);
    if (input.NoKeysActive()) {
        return;
    }

    uint32_t sequence = mCurrentCommandSequence++;
    mCommands.emplace_back(sequence, input);
}

void Client::SendCommandsToServer() const {
    if (mState != ClientState::CLIENT_CONNECTED || mConnecting || mDisconnecting) {
        return;
    }

    if (mCommands.empty()) {
        ClientOperations::sendPingToServer(this);
        return;
    }

    ClientOperations::sendDataToServer(this);
}

void Client::ReceiveStateFromServer()  {
    if (mState != ClientState::CLIENT_CONNECTED) {
        return;
    }

    // receive server response
    if (!ClientOperations::receiveDataPacketFromServer(this)) {
        return;
    }

    if (!mRawState.active) {
       mGame->Quit();
    }

    // set the player if not set yet
    if (!mGame->IsPlayerSet()) {
        mGame->SetPlayer(Vector2(mRawState.posX, mRawState.posY), mRawState.rotation);
        return;
    }

    // remove commands already confirmed by the server
    if (mLastReceivedInputSequence > mLasRemovedInputSequence) {
        CleanConfirmedCommands(mLastReceivedInputSequence);
    }

    // set the state sent by the server
    mGame->SetPlayerState(mRawState);

    // apply again the rest of the commands
    ReprocessLocalState();

    // control enemies state
    for (const auto other: mOtherStates) {
        if (!mGame->IsEnemySet(other.id)) {
            mGame->SetEnemy(other.id, Vector2(other.posX, other.posY), other.rotation);
        }else {
            mGame->SetEnemiesState(mOtherStates);
        }
    }
}

void Client::CleanConfirmedCommands(uint32_t confirmedSequence) {
    const auto it = std::find_if(
        mCommands.begin(),
        mCommands.end(),
        [&confirmedSequence](const Command& cmd) {
            return cmd.sequence > confirmedSequence;
        }
    );

    if (it != mCommands.begin()) {
        mCommands.erase(mCommands.begin(), it);
        mLasRemovedInputSequence = confirmedSequence;
    }
}

void Client::ReprocessLocalState() const {
    if (mCommands.empty()) {
        return;
    }

    for (const auto &cmd : mCommands) {
        const auto state = SDLInputParser::revert(cmd.inputData);

        mGame->GetPlayer()->ProcessInput(state);
        mGame->GetPlayer()->Update(Game::SIM_DELTA_TIME);
    }
}

ConnectionStatus Client::CheckConnection() {
    if (!mConnectionFuture.valid()) {
        return ConnectionStatus::FAILURE;
    }

    if (mConnectionFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        const bool result = mConnectionFuture.get();

        mConnectionFuture = {};

        if (!result) {
            return ConnectionStatus::FAILURE;
        };
        return ConnectionStatus::SUCCESS;
    }
    return ConnectionStatus::IN_PROGRESS;
}
