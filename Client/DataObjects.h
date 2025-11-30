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

struct OtherState {
    float posX, posY;

    OtherState() : posX(0), posY(0) {}
};

struct FullState {
    RawState rawState;
    OtherState otherState;
    uint32_t lastConfirmedInputSequence;

    FullState(const RawState &raw, const OtherState &other, const uint32_t sequence)
    :rawState(raw), otherState(other), lastConfirmedInputSequence(sequence) {}
};

struct GameState {
    RawState rawState;
    OtherState otherState;

    explicit GameState(const RawState &raw, const OtherState &other)
    :rawState(raw), otherState(other) {}
};