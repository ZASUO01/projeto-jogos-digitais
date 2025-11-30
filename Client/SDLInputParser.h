//
// Created by pedro-souza on 29/11/2025.
//

#pragma once
#include <SDL.h>
#include "InputData.h"

namespace SDLInputParser {
    InputData parse(const Uint8 *keyState);
    const Uint8 *revert(const InputData &input);
};
