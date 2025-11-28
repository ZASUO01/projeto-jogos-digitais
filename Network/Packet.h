//
// Created by pedro-souza on 23/11/2025.
//
#pragma once

#include <cstdint>
#include <cstdlib>
#include "Platforms.h"

#pragma pack(1)
class Packet {
public:
    Packet();
    Packet(uint16_t _sequence, uint8_t _flag, uint32_t _nonce);
    void SetData(const void *sourceData, size_t dataSize);
    void BuildPacket();
    [[nodiscard]] bool IsValid() const;
    void PrintPacket() const;

    [[nodiscard]] const void* GetData() const { return data; }
    [[nodiscard]] uint8_t GetFlag() const { return flag; }

    [[nodiscard]] uint16_t GetSequence() const {
        if (state == PACKET_READY) {
            return ntohs(sequence);
        }
        return sequence;
    }

    [[nodiscard]] uint32_t GetNonce() const {
        if (state == PACKET_READY) {
            return ntohl(nonce);
        }
        return nonce;
    }

    [[nodiscard]] uint16_t GetLength() const {
        if (state == PACKET_READY) {
            return ntohs(length);
        }
        return length;
    }

    static constexpr size_t MAX_PACKET_DATA_BYTES = 1024;
    static constexpr size_t PACKET_HEADER_BYTES = 20;

    static constexpr uint8_t SYN_FLAG = 0x01;
    static constexpr uint8_t SYN_ACK_FLAG = 0x02;
    static constexpr uint8_t ACK_FLAG = 0x03;
    static constexpr uint8_t DATA_FLAG = 0x04;
    static constexpr uint8_t END_FLAG = 0x05;
    static constexpr uint8_t END_ACK_FLAG = 0x06;
    static constexpr uint8_t RST_FLAG = 0x07;

private:
    uint32_t sync1;
    uint32_t sync2;
    uint8_t state;
    uint16_t sequence;
    uint8_t flag;
    uint32_t nonce;
    uint16_t length;
    uint16_t checksum;
    uint8_t data[MAX_PACKET_DATA_BYTES];

    static constexpr uint32_t PACKET_SYNC_BYTES = 0x554E4554;
    static constexpr uint8_t PACKET_HOLD = 1;
    static constexpr uint8_t PACKET_READY = 2;
};
#pragma pack(0)