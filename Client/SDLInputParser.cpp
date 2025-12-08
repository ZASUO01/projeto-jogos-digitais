//
// Created by pedro-souza on 29/11/2025.
//

#include "SDLInputParser.h"
#include <map>
#include <array>

static const std::map<KeyValue, SDL_Scancode> KeyToScancodeMap = {
    {KeyValue::MOVE_FORWARD, SDL_SCANCODE_W},
    {KeyValue::MOVE_LEFT, SDL_SCANCODE_A},
    {KeyValue::MOVE_BACKWARD, SDL_SCANCODE_S},
    {KeyValue::MOVE_RIGHT, SDL_SCANCODE_D}
};

static constexpr size_t SIMULATED_KEYSTATE_SIZE = 300;

InputData SDLInputParser::parse(const Uint8 *keyState) {
    InputData input;
    input.ResetKeys();

    if (keyState[SDL_SCANCODE_W]) {
        input.SetKeyActive(KeyValue::MOVE_FORWARD);
    }

    if (keyState[SDL_SCANCODE_A]) {
        input.SetKeyActive(KeyValue::MOVE_LEFT);
    }

    if (keyState[SDL_SCANCODE_S]) {
        input.SetKeyActive(KeyValue::MOVE_BACKWARD);
    }

    if (keyState[SDL_SCANCODE_D]) {
        input.SetKeyActive(KeyValue::MOVE_RIGHT);
    }

    if (keyState[SDL_SCANCODE_SPACE]) {
        input.SetKeyActive(KeyValue::SHOOT);
    }

    return input;
}

const Uint8 *SDLInputParser::revert(const InputData &input) {
    static std::array<Uint8, SIMULATED_KEYSTATE_SIZE> keyStateSimulation;
    keyStateSimulation.fill(0);

    for (const auto&[fst, snd] : KeyToScancodeMap) {
        const KeyValue gameKey = fst;

        if (const SDL_Scancode sdlScancode = snd; sdlScancode < SIMULATED_KEYSTATE_SIZE && input.IsKeyActive(gameKey)) {
            keyStateSimulation[sdlScancode] = 1;
        }
    }

    return keyStateSimulation.data();
}