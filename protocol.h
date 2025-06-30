// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

struct PersonData 
{
    char surname[64];      // фамилия
    char name[64];         // имя
    char patronymic[64];   // отчество
    uint8_t age;           // возраст
    uint16_t weight;       // вес
};

const uint8_t DELIMITER = 0xAA;      // разделитель между полями
const uint8_t PACKET_START = 0x55;   // маркер начала пакета
const uint8_t PACKET_END = 0xAA;     // маркер конца пакета

// функция вычисления контрольной суммы
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

// отправка одного поля с контольной суммой и разделителем
bool sendField(SOCKET socket, const void* data, size_t size)
{
    // отправляем данные
    if (send(socket, static_cast<const char*>(data), size, 0) != size) 
    {
        std::cerr << "Failed to send data" << std::endl;
        return false;
    }

    // отправляем контрольную сумму
    uint8_t crc = calculateCRC(data, size);
    if (send(socket, reinterpret_cast<const char*>(&crc), sizeof(crc), 0) != sizeof(crc)) 
    {
        std::cerr << "Failed to send CRC" << std::endl;
        return false;
    }

    // отправляем разделитель
    if (send(socket, reinterpret_cast<const char*>(&DELIMITER), sizeof(DELIMITER), 0) != sizeof(DELIMITER)) 
    {
        std::cerr << "Failed to send delimiter" << std::endl;
        return false;
    }

    return true;
}

bool receiveField(SOCKET socket, void* buffer, size_t size) 
{
    // прием данных
    if (recv(socket, static_cast<char*>(buffer), size, 0) != size) 
    {
        std::cerr << "Failed to receive data" << std::endl;
        return false;
    }

    // прием контрольной суммы
    uint8_t receivedCRC;
    if (recv(socket, reinterpret_cast<char*>(&receivedCRC), sizeof(receivedCRC), 0) != sizeof(receivedCRC)) 
    {
        std::cerr << "Failed to receive CRC" << std::endl;
        return false;
    }

    // проверка контрольной суммы
    uint8_t calculatedCRC = calculateCRC(buffer, size);
    if (receivedCRC != calculatedCRC)
    {
        std::cerr << "CRC mismatch" << std::endl;
        return false;
    }

    // проверка разделителя
    uint8_t delimiter;
    if (recv(socket, reinterpret_cast<char*>(&delimiter), sizeof(delimiter), 0) != sizeof(delimiter))
    {
        std::cerr << "Failed to receive delimiter" << std::endl;
        return false;
    }

    if (delimiter != DELIMITER)
    {
        std::cerr << "Invalid delimiter" << std::endl;
        return false;
    }

    return true;
}

#endif // PROTOCOL_H