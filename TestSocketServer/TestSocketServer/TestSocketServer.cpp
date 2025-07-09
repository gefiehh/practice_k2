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

    cout << "Server is runnung and expects connection..." << endl;

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

    while (true)
    {
        PersonData person;
        if (!receivePersonData(ClientSocket, person))
        {
            cerr << "Error when receiving data. Closing connection." << endl;
            break;
        }

        // вывод полученных данных
        cout << "\nПолучена информация о человеке:" << endl;
        cout << "Фамилия: " << person.surname << endl;
        cout << "Имя: " << person.name << endl;
        cout << "Отчество: " << person.patronymic << endl;
        cout << "Возраст: " << static_cast<int>(person.age) << endl;
        cout << "Вес: " << person.weight << endl;

        cout << "\nWaiting for the next package..." << endl;
    }

    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
