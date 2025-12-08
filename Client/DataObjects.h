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
    bool active;
    float posX, posY, rotation;

    RawState() : active(false), posX(0), posY(0), rotation(0) {}
};

struct OtherState {
    int id;
    float posX, posY, rotation;
    bool hasShot;

    OtherState(const int id, const float x, const float y, const float rot, const bool hasShot)
    :id(id), posX(x), posY(y), rotation(rot), hasShot(hasShot) {}
    OtherState() :id(-1), posX(0), posY(0), rotation(0), hasShot(false) {}
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

struct InterpolationState {
    float targetPosX, targetPosY, targetRotation;
};