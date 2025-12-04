//
// Created by pedro-souza on 30/11/2025.
//

#pragma once
#include <cstdint>
#include <vector>

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
    int id;
    float posX, posY, rotation;

    OtherState(const int id, const float x, const float y, const float rot) :id(id), posX(x), posY(y), rotation(rot) {}
    OtherState() :id(-1), posX(0), posY(0), rotation(0) {}
};

#define MAX_OTHER_STATES 3

struct FullState {
    RawState rawState;
    OtherState otherStates[MAX_OTHER_STATES];
    size_t otherStateSize;
    uint32_t lastConfirmedInputSequence;

    FullState(const RawState &raw,const uint32_t sequence)
    :rawState(raw), otherStateSize(0), lastConfirmedInputSequence(sequence) {}
};

struct GameState {
    RawState rawState;
    std::vector<OtherState> otherStates;

    explicit GameState(const RawState &raw, const OtherState others[MAX_OTHER_STATES], size_t otherStateSize)
    :rawState(raw) {
        const size_t minSize = std::min(otherStateSize, static_cast<size_t>(MAX_OTHER_STATES));
        otherStates.assign(others, others + minSize);
    }
};