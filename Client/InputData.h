//
// Created by pedro-souza on 26/11/2025.
//
#pragma once
#include <cstdint>

enum class KeyValue : uint8_t{
    NONE = 0,
    MOVE_FORWARD = 1 << 0, // 0000 0001
    MOVE_BACKWARD = 1 << 1, // 0000 0010
    MOVE_LEFT = 1 << 2, // 0000 0100
    MOVE_RIGHT = 1 << 3, // 0000 1000
    SHOOT = 1 << 4, // 0001 0000
};

class InputData {
public:
    InputData() :mActiveKeys(0){}
    ~InputData() = default;

    void SetKeyActive(KeyValue key) {
        mActiveKeys |= static_cast<uint8_t>(key);
    }

    [[nodiscard]] bool IsKeyActive(KeyValue key) const {
        return (mActiveKeys & static_cast<uint8_t>(key)) != 0;
    }

    void SetKeyInactive(KeyValue key) {
        mActiveKeys &= ~static_cast<uint8_t>(key);
    }

    void ResetKeys() { mActiveKeys = 0; }

    [[nodiscard]] bool NoKeysActive() const { return mActiveKeys == 0; }
private:
    uint8_t mActiveKeys;
};