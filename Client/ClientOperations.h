//
// Created by pedro-souza on 24/11/2025.
//

#pragma once

#include "Client.h"

namespace ClientOperations {
    void sendSinglePacketToServer(const Client *client, uint8_t flag);
    bool receiveSinglePacketFromServer(Client *client, uint8_t flag);
    void sendDataToServer(const Client *client);
    bool receiveDataPacketFromServer(Client *client);
};