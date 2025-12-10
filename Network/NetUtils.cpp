//
// Created by pedro-souza on 23/11/2025.
//

#include "NetUtils.h"
#include <cstring>
#include <ctime>
#include "../Source/Random.h"

uint16_t NetUtils::getNetChecksum(const void *data, const size_t dataSize){
    const auto *buf = static_cast<const uint8_t *>(data);
    uint32_t sum = 0;

    for (size_t i = 0; i + 1 < dataSize; i += 2) {
        uint16_t word;
        memcpy(&word, buf + i, sizeof(uint16_t));
        sum += word;
    }

    if(dataSize % 2 != 0){
        sum += static_cast<uint16_t>(buf[dataSize - 1] << 8);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return static_cast<uint16_t>(~sum);
}

uint32_t NetUtils::getNonce() {
    return static_cast<uint32_t>(std::time(nullptr));
}

uint32_t NetUtils::getRandomNonce(const uint32_t baseNonce) {
    const uint32_t randomFactor = Random::GetIntRange(0, static_cast<int>(baseNonce));

    return baseNonce ^ randomFactor;
}