//
// Created by pedro-souza on 30/11/2025.
//

#pragma once
#include <cstdint>

#include "InputData.h"

struct InterpolationState {
    float targetPosX, targetPosY, targetRotation;
};

#pragma pack(1)
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
    int life;
    float invulnerableTimer;

    RawState() : active(false), posX(0), posY(0), rotation(0), life(0), invulnerableTimer(0) {}
};

struct OtherState {
    int id;
    float posX, posY, rotation;
    bool hasShot;
    int life;
    float invulnerableTimer;

    OtherState(
        const int id,
        const float x,
        const float y,
        const float rot,
        const bool hasShot,
        const int life,
        const float invulnerableTimer
    )
    :id(id), posX(x), posY(y), rotation(rot), hasShot(hasShot), life(life), invulnerableTimer(invulnerableTimer) {}
    OtherState() :id(-1), posX(0), posY(0), rotation(0), hasShot(false), life(0), invulnerableTimer(0) {}
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
#pragma pack(0)