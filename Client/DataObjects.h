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
    float posX, posY, rotation;

    RawState() : posX(0), posY(0), rotation(0) {}
};

struct OtherState {
    float posX, posY;

    OtherState() : posX(0), posY(0) {}
};

struct FullState {
    RawState rawState;
    uint32_t lastConfirmedInputSequence;

    FullState(const RawState &raw, const uint32_t sequence)
    :rawState(raw), lastConfirmedInputSequence(sequence) {}
};

struct GameState {
    RawState rawState;

    explicit GameState(const RawState &raw)
    :rawState(raw) {}
};