//
// Created by pedro-souza on 24/11/2025.
//

#pragma once

#include "Client.h"

namespace ClientOperations {
    void sendSynToServer(const Client *client);
    bool receiveSynAckFromServer(Client *client);
    void sendAckToServer(const Client *client);
    void sendDataToServer();
};