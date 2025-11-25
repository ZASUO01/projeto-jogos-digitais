//
// Created by pedro-souza on 23/11/2025.
//
#pragma once

#include <cstdint>
#include <cstdlib>

namespace  NetUtils {
    uint16_t getNetChecksum(const void *data, size_t dataSize);
};
