//
// Created by pedro-souza on 23/11/2025.
//

#include "NetUtils.h"
#include <cstring>

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