// СИНХРОННАЯ РАБОТА
// В свойствах решения: 1)компановщик -- ввод -- доп.зависимости: ws2_32.lib;
//						2)общ.свойства конфицурации -- набор символов:не задано
// на основе кода клиента

#define WIN32_LEAN_AND_MEAN  

#include <iostream>
#include <Windows.h>  
#include <WinSock2.h> 
#include <WS2tcpip.h>  
#include "protocol.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");

    WSADATA wsaData;
    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;
    SOCKET ClientSocket = INVALID_SOCKET; 
    SOCKET ListenSocket = INVALID_SOCKET; 

    int result;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        cout << "WSAStartup failed, result = " << result << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // хотим получить инфу для пассивной стороны

    result = getaddrinfo(NULL, "111", &hints, &addrResult); // NULL т.к. сервер не знает адрес
    if (result != 0)
    {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        cout << "Binding socket failed" << endl;
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        cout << "Listening socket failed" << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        cout << "Accepting socket failed" << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    // получение данных
    uint8_t startMarker;
    if (recv(ClientSocket, reinterpret_cast<char*>(&startMarker), sizeof(startMarker), 0) != sizeof(startMarker))
    {
        std::cerr << "Failed to receive start marker" << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (startMarker != PACKET_START)
    {
        std::cerr << "Invalid start marker" << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    PersonData person;
    if (!receiveField(ClientSocket, person.surname, sizeof(person.surname)))
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (!receiveField(ClientSocket, person.name, sizeof(person.name)))
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (!receiveField(ClientSocket, person.patronymic, sizeof(person.patronymic)))
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (!receiveField(ClientSocket, &person.age, sizeof(person.age)))
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (!receiveField(ClientSocket, &person.weight, sizeof(person.weight)))
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // проверка байта конца
    uint8_t endMarker;
    if (recv(ClientSocket, reinterpret_cast<char*>(&endMarker), sizeof(endMarker), 0) != sizeof(endMarker))
    {
        std::cerr << "Failed to receive end marker" << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    if (endMarker != PACKET_END)
    {
        std::cerr << "Invalid end marker" << std::endl;
    }

    // вывод полученных данных
    std::cout << "Информация о человеке:" << std::endl;
    std::cout << "Фамилия: " << person.surname << std::endl;
    std::cout << "Имя: " << person.name << std::endl;
    std::cout << "Отчество: " << person.patronymic << std::endl;
    std::cout << "Возраст: " << static_cast<int>(person.age) << std::endl;
    std::cout << "Вес: " << person.weight << std::endl;

    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
