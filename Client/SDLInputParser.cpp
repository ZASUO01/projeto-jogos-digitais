//
// Created by pedro-souza on 29/11/2025.
//

#include "SDLInputParser.h"

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

    return input;
}