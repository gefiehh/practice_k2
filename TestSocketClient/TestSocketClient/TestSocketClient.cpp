// СИНХРОННАЯ РАБОТА
// В свойствах решения: 1)компановщик -- ввод -- доп.зависимости: ws2_32.lib;
//						2)общ.свойства конфицурации -- набор символов:не задано

#define WIN32_LEAN_AND_MEAN  
// макрос для сетевого программирования

#include <iostream>
#include <Windows.h>  // работа с виндой
#include <WinSock2.h>  // работа с сокетами
#include <WS2tcpip.h>  // работа по tcp/ip протоколу

#include "protocol.h"

using namespace std;

int main()
{
	setlocale(LC_ALL, "Russian");

	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ConnectSocket = INVALID_SOCKET;

	int result; // 1 - программа отработала с ошибкой, 0 - программа отработала без ошибки

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);  // MAKEWORD(2, 2) - макрос, последняя версия 2.2
	if (result != 0)
	{
		cout << "WSAStartup failed, result = " << result << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));  // зануляем все поля структуры
	hints.ai_family = AF_INET; // семейство протаколов. AF_INET - 4 байтный изернет
	hints.ai_socktype = SOCK_STREAM; // типы сокеты, которые хотим получить. SOCK_STREAM - потоковые сокеты
	hints.ai_protocol = IPPROTO_TCP;  // вид протокола. IPPROTO_TCP - tcp/ip

	// получаем информацию о адресе
	result = getaddrinfo("localhost", "111", &hints, &addrResult);
	if (result != 0)
	{
		cout << "getaddrinfo failed with error: " << result << endl;
		WSACleanup();
		return 1;
	}

	// создание сокета
	ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol); // создание сокета
	if (ConnectSocket == INVALID_SOCKET)
	{
		cout << "Socket creation failed" << endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	// подключение к серверу
	result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen); // создание соединения
	if (result == SOCKET_ERROR)
	{
		cout << "Unable connect to server" << endl;
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

    // подготовка данных
    PersonData person;
    memset(&person, 0, sizeof(PersonData));
    strncpy_s(person.surname, "Иванов", sizeof(person.surname));
    strncpy_s(person.name, "Иван", sizeof(person.name));
    strncpy_s(person.patronymic, "Иванович", sizeof(person.patronymic));
    person.age = 30;
    person.weight = 75;

    // отправка данных
    // 1. байт начала пакета
    send(ConnectSocket, reinterpret_cast<const char*>(&PACKET_START), sizeof(PACKET_START), 0);

    // 2. отправка полей
    sendField(ConnectSocket, person.surname, sizeof(person.surname));
    sendField(ConnectSocket, person.name, sizeof(person.name));
    sendField(ConnectSocket, person.patronymic, sizeof(person.patronymic));
    sendField(ConnectSocket, &person.age, sizeof(person.age));
    sendField(ConnectSocket, &person.weight, sizeof(person.weight));

    // 3. байт конца пакета
    send(ConnectSocket, reinterpret_cast<const char*>(&PACKET_END), sizeof(PACKET_END), 0);

    // завершение работы
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}
