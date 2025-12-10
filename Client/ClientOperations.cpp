//
// Created by pedro-souza on 24/11/2025.
//

#include "ClientOperations.h"
#include "../Network/Packet.h"
#include "../Network/Socket.h"

void ClientOperations::sendSinglePacketToServer(const Client *client, const uint8_t flag) {
    if (flag != Packet::SYN_FLAG && flag != Packet::ACK_FLAG && flag != Packet::END_FLAG) {
        return;
    }

    Packet packet(
        client->GetCurrentPacketSequence(),
        flag,
        client->GetClientNonce()
    );
    packet.BuildPacket();

    constexpr size_t packetSize = Packet::PACKET_HEADER_BYTES;
    sockaddr_in addr = client->GetServerAddress();

    SocketUtils::sendPacketToV4(
        client->GetSocket(),
        &packet,
        packetSize,
        &addr
    );
}

bool ClientOperations::receiveSinglePacketFromServer(Client *client, const uint8_t flag) {
    if (flag != Packet::SYN_ACK_FLAG && flag != Packet::END_ACK_FLAG) {
        return false;
    }

    if (!SocketUtils::socketReadyToReceive(
        client->GetSocket(), Client::CONNECTION_RECEIVING_TIMEOUT_IN_MS)) {
        return false;
    }

    Packet packet;
    sockaddr_in addr = client->GetServerAddress();

    if (!SocketUtils::receivePacketFromV4(
        client->GetSocket(),
        &packet,
        &addr
    )) {
        return false;
    }

    if (!packet.IsValid()) {
        return false;
    }

    if (packet.GetFlag() != flag) {
        constexpr int maxTrials = 100;
        int trials = 0;

        while (trials < maxTrials) {
            trials++;
            if (!SocketUtils::socketReadyToReceive(client->GetSocket(), 0)) {
               return false;
            }

            Packet temp;

            if (!SocketUtils::receivePacketFromV4(
                client->GetSocket(),
                &temp,
                &addr
            )) {
                return false;
            }

            if (!temp.IsValid()) {
                return false;
            }

            if (temp.GetFlag() == flag) {
                break;
            }
        }
    }

    if (packet.GetSequence() != client->GetCurrentPacketSequence() + 1) {
        return false;
    }

    client->SetNonce(packet.GetNonce());
    client->IncreasePacketSequence();

    return true;
}

void ClientOperations::sendDataToServer(const Client *client) {
    Packet packet(
        client->GetCurrentPacketSequence(),
        Packet::DATA_FLAG,
        client->GetClientNonce()
    );

    const std::vector<Command>& commands = client->GetCommands();
    const size_t commandsSize = commands.size() * sizeof(Command);
    packet.SetData(commands.data(), commandsSize);
    packet.BuildPacket();

    const size_t packetSize = Packet::PACKET_HEADER_BYTES + packet.GetLength();
    sockaddr_in addr = client->GetServerAddress();

    SocketUtils::sendPacketToV4(
        client->GetSocket(),
        &packet,
        packetSize,
        &addr
    );
}

bool ClientOperations::receiveDataPacketFromServer(Client *client) {
    if (!SocketUtils::socketReadyToReceive(
        client->GetSocket(), 0)) {
        return false;
    }

    Packet packet;
    sockaddr_in addr = client->GetServerAddress();

    if (!SocketUtils::receivePacketFromV4(
        client->GetSocket(),
        &packet,
        &addr
    )) {
        return false;
    }

    if (!packet.IsValid()) {
        return false;
    }

    if (packet.GetFlag() != Packet::DATA_FLAG) {
        return false;
    }

    // extract state
    const auto state = static_cast<const FullState*>(packet.GetData());

    const uint32_t received = state->lastConfirmedInputSequence;
    client->SetLastReceivedInputSequence(received);

    client->SetOtherState(state->otherStates, state->otherStateSize);

    client->SetRawState(state->rawState);
    return true;
}

void ClientOperations::sendPingToServer(const Client *client) {
    Packet packet(
        client->GetCurrentPacketSequence(),
        Packet::PING_FLAG,
        client->GetClientNonce()
    );
    packet.BuildPacket();

    constexpr size_t packetSize = Packet::PACKET_HEADER_BYTES;
    sockaddr_in addr = client->GetServerAddress();

    SocketUtils::sendPacketToV4(
        client->GetSocket(),
        &packet,
        packetSize,
        &addr
    );
}