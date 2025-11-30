//
// Created by pedro-souza on 30/11/2025.
//

#pragma once
#include <cstdint>
#include "InputData.h"

// Commands to be sent to the server
struct Command {
    uint32_t sequence;
    InputData inputData;

    Command(const uint32_t sequence, const InputData &inputData)
    :sequence(sequence), inputData(inputData) {}
};

struct RawState {
    float posX, posY;

    RawState() : posX(0), posY(0) {}
};

struct FullState {
    RawState rawState;
    uint32_t lastConfirmedInputSequence;

    FullState(const RawState &raw, const uint32_t sequence)
    :rawState(raw), lastConfirmedInputSequence(sequence) {}
};