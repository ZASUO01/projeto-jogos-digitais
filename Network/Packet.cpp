//
// Created by pedro-souza on 23/11/2025.
//

#include "Packet.h"
#include "Platforms.h"
#include "NetUtils.h"
#include <cstring>
#include <cstdio>

Packet::Packet()
:sync1(PACKET_SYNC_BYTES)
,sync2(PACKET_SYNC_BYTES)
,state(PACKET_HOLD)
,sequence(0)
,flag(0)
,nonce(0)
,length(0)
,checksum(0)
,data{}
{}

Packet::Packet(const uint16_t _sequence, const uint8_t _flag, const uint32_t _nonce)
:sync1(PACKET_SYNC_BYTES)
,sync2(PACKET_SYNC_BYTES)
,state(PACKET_HOLD)
,sequence(_sequence)
,flag(_flag)
,nonce(_nonce)
,length(0)
,checksum(0)
,data{}
{}

void Packet::SetData(const void *sourceData, const size_t dataSize) {
    if (state != PACKET_HOLD) {
        return;
    }

    if (sourceData == nullptr) {
        return;
    }

    if (dataSize > MAX_PACKET_DATA_BYTES) {
        return;
    }

    if (dataSize > 0) {
        memcpy(data, sourceData, dataSize);
    }

    length = dataSize;
}

void Packet::BuildPacket() {
    if (state != PACKET_HOLD) {
        return;
    }

    sync1 = htonl(sync1);
    sync2 = htonl(sync2);
    sequence = htons(sequence);
    nonce = htonl(nonce);

    state = PACKET_READY;

    const uint16_t _length = length;
    length = htons(length);

    const size_t packetSize = PACKET_HEADER_BYTES + _length;
    checksum = NetUtils::getNetChecksum(this, packetSize);
    checksum = htons(checksum);
}

bool Packet::IsValid() const {
    if (state != PACKET_READY) {
        return false;
    }

    if(sync1 != htonl(PACKET_SYNC_BYTES) ||
       sync2 != htonl(PACKET_SYNC_BYTES)) {
        return false;
       }

    const uint16_t dataSize = ntohs(length);
    if(dataSize > MAX_PACKET_DATA_BYTES){
        return false;
    }

    const uint16_t savedChecksum = checksum;
    const auto temp = const_cast<Packet*>(this);
    temp->checksum = 0;

    const size_t packetSize = PACKET_HEADER_BYTES + dataSize;
    const uint16_t calculatedChecksum = NetUtils::getNetChecksum(temp, packetSize);
    temp->checksum = savedChecksum;

    return calculatedChecksum == ntohs(savedChecksum);
}

void Packet::PrintPacket() const {
    if (state != PACKET_READY) {
        return;
    }

    const uint16_t _length = ntohs(length);

    printf("==== PACKET HEADER ====\n");
    printf("SEQUENCE: %hu\n", ntohs(sequence));
    printf("FLAG:   %hhu\n", flag);
    printf("NONCE:     %u\n", ntohl(nonce));
    printf("LENGTH: %hu\n", _length);
    printf("==== CONTROL FIELDS ====\n");
    printf("SYNC1:    %#x\n", ntohl(sync1));
    printf("SYNC2:    %#x\n", ntohl(sync2));
    printf("CHECKSUM: %#x\n", ntohl(checksum));
}