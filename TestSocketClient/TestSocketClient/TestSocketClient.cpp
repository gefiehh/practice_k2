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

	// подготовка нескольких пакетов данных
	PersonData persons[] = {
		{"Иванов", "Иван", "Иванович", 30, 75},
		{"Петров", "Петр", "Петрович", 25, 80}
	};
	const int personsCount = sizeof(persons) / sizeof(PersonData);

	// вывод меню
	// обработка выбора
	// отправка данных
	while (true) {
		cout << "Avaliable data packs:" << endl;
		for (int i = 0; i < personsCount; i++) {
			cout << i + 1 << ". " << persons[i].surname << " " << persons[i].name
				<< " " << persons[i].patronymic << endl;
		}
		cout << "0. Выход" << endl;
		cout << "Выберите пакет для отправки (0-" << personsCount << "): ";

		int choice;
		cin >> choice;
		cin.ignore(); // очистка буфера ввода

		if (choice == 0) break;
		if (choice < 1 || choice > personsCount) {
			cout << "Wrong choise" << endl;
			continue;
		}

		// отправка выбранного пакета
		sendPersonData(ConnectSocket, persons[choice - 1]);
		cout << "Data packet sent" << endl;
	}

    // завершение работы
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}


// отправлять структуру целиком (отправляем в бинарном виде, получаем в виде структуры)
// проверка если больше 64 байт или пустая строка
// асинхронная работа сокетов + предача нескольких пакетов 
