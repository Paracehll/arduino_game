#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <stdint.h>

// Packet format: [HEADER, TYPE, DATA, CHECKSUM]
#define PACKET_HEADER 0xAA
#define TYPE_SPAWN_ENEMY 0x01

struct Packet {
    uint8_t header;
    uint8_t type;
    uint8_t data;
    uint8_t checksum;
};

void sendEnemyTransfer(uint8_t x_percent) {
    uint8_t checksum = PACKET_HEADER + TYPE_SPAWN_ENEMY + x_percent;
    Serial.write(PACKET_HEADER);
    Serial.write(TYPE_SPAWN_ENEMY);
    Serial.write(x_percent);
    Serial.write(checksum);
}

bool receivePacket(Packet &p) {
    if (Serial.available() >= 4) {
        if (Serial.read() == PACKET_HEADER) {
            p.header = PACKET_HEADER;
            p.type = Serial.read();
            p.data = Serial.read();
            p.checksum = Serial.read();

            uint8_t calculatedChecksum = p.header + p.type + p.data;
            if (calculatedChecksum == p.checksum) {
                return true;
            }
        }
    }
    return false;
}

#endif
