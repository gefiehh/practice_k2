// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <iostream>
#include <cstring>

using namespace std;

struct PersonData
{
    char surname[64];      // �������
    char name[64];         // ���
    char patronymic[64];   // ��������
    uint8_t age;           // �������
    uint16_t weight;       // ���
};

const uint8_t PACKET_START = 0x55;   // ������ ������ ������
const uint8_t PACKET_END = 0xAA;     // ������ ����� ������

// ������� ���������� ����������� �����
uint8_t calculateCRC(const void* data, size_t size) 
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint8_t crc = 0;
    for (size_t i = 0; i < size; i++) 
    {
        crc ^= bytes[i];
    }
    return crc;
}

void sendPersonData(SOCKET socket, const PersonData& person) {
    // 1. ���� ������ ������
    send(socket, reinterpret_cast<const char*>(&PACKET_START), sizeof(PACKET_START), 0);

    // 2. �������� ���� ��������� �������
    send(socket, reinterpret_cast<const char*>(&person), sizeof(PersonData), 0);

    // 3. �������� ����������� �����
    uint8_t crc = calculateCRC(&person, sizeof(PersonData));
    send(socket, reinterpret_cast<const char*>(&crc), sizeof(crc), 0);

    // 4. ���� ����� ������
    send(socket, reinterpret_cast<const char*>(&PACKET_END), sizeof(PACKET_END), 0);
}

// ��������� ������
bool receivePersonData(SOCKET socket, PersonData& person) {
    uint8_t startMarker;
    if (recv(socket, reinterpret_cast<char*>(&startMarker), sizeof(startMarker), 0) != sizeof(startMarker))
    {
        cerr << "Failed to receive start marker" << endl;
        return false;
    }

    if (startMarker != PACKET_START)
    {
        cerr << "Invalid start marker" << endl;
        return false;
    }

    if (recv(socket, reinterpret_cast<char*>(&person), sizeof(PersonData), 0) != sizeof(PersonData))
    {
        cerr << "Failed to receive person data" << endl;
        return false;
    }

    uint8_t receivedCRC;
    if (recv(socket, reinterpret_cast<char*>(&receivedCRC), sizeof(receivedCRC), 0) != sizeof(receivedCRC))
    {
        cerr << "Failed to receive CRC" << endl;
        return false;
    }

    uint8_t calculatedCRC = calculateCRC(&person, sizeof(PersonData));
    if (receivedCRC != calculatedCRC)
    {
        cerr << "CRC mismatch" << endl;
        return false;
    }

    uint8_t endMarker;
    if (recv(socket, reinterpret_cast<char*>(&endMarker), sizeof(endMarker), 0) != sizeof(endMarker))
    {
        cerr << "Failed to receive end marker" << endl;
        return false;
    }

    if (endMarker != PACKET_END)
    {
        cerr << "Invalid end marker" << endl;
        return false;
    }

    return true;
}

#endif // PROTOCOL_H