// server.cpp: - серверная часть (TCP)

/*  Для того, чтобы сокеты заработали под Windows,
 *  необходимо при написании программы пройти следующие этапы:
 *
 * 0. Подключение всех необходимых библиотек Win32API для работы с сокетами
 *
 * 1. Инициализация сокетных интерфейсов Win32API.
 *
 * 2. Инициализация сокета, т.е. создание специальной структуры данных
 *    и её инициализация вызовом функции.
 *
 * 3. «Привязка» созданного сокета к конкретной паре IP-адрес/Порт
 *    – с этого момента данный сокет (его имя) будет ассоциироваться
 *    с конкретным процессом, который «висит» по указанному адресу и порту.
 *
 * 4. Для серверной части приложения:
 *    запуск процедуры «прослушки» подключений на привязанный сокет.
 *
 *    Для клиентской части приложения:
 *    запуск процедуры подключения к серверному сокету (должны знать его IP-адрес/Порт).
 *
 * 5. Акцепт / Подтверждение подключения (обычно на стороне сервера).
 *
 * 6. Обмен данными между процессами через установленное сокетное соединение.
 *
 * 7. Закрытие сокетного соединения.
 */

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// Maximum size of buffer for exchange info between server and client
const short BUFFER_SIZE = 1024;

int main()                  // SERVER
{
    const char IP_SERV[] = "127.0.0.1";          // Enter local Server IP address
    const int PORT_NUM = 80;             // Enter Open working server port

    // 1. Инициализация сокетных интерфейсов Win32API.
    WSADATA wsData;

    // return 0 - всё хорошо
    int error_1_init = WSAStartup(MAKEWORD(2, 2),
                                  &wsData);

    if (error_1_init != 0)
    {
        std::cout << "Error WinSock version inicialization #";
        std::cout << WSAGetLastError();

        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "WinSock initialization is OK" << std::endl;
    }


    /* 2. Инициализация сокета, т.е.создание специальной структуры данных
     *    и её инициализация вызовом функции.
     */
    SOCKET ServSock = socket(AF_INET,
                             SOCK_STREAM,
                             0);

    if (ServSock == INVALID_SOCKET)
    {
        std::cout << "Error initialization Server socket #"
                  << WSAGetLastError() << std::endl;

        closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета
        WSACleanup();           // деинициализировать сокет
        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Server socket initialization is OK" << std::endl;
    }


    /* 3. «Привязка» созданного сокета к конкретной паре IP-адрес/Порт -
     *    с этого момента данный сокет(его имя) будет ассоциироваться
     *    с конкретным процессом, который «висит» по указанному адресу и порту.
     */
    in_addr ip_to_num;
    int error_2_IP_translation = inet_pton(AF_INET,
                                           IP_SERV,
                                           &ip_to_num);

    if (error_2_IP_translation <= 0)
    {
        std::cout << "Error in IP translation to special numeric format" << std::endl;
        return 1;
    }

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr   = ip_to_num;
    servInfo.sin_port   = htons(PORT_NUM);


    int error_3_IP_bind = bind(ServSock,
                               reinterpret_cast<sockaddr*>(&servInfo),
                               sizeof(servInfo));

    if (error_3_IP_bind != 0)
    {
        std::cout << "Error Socket binding to server info. Error #"
                  << WSAGetLastError() << std::endl;

        closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета
        WSACleanup();           // деинициализировать сокет
        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Binding socket to Server info is OK" << std::endl;
    }


    /* Этап 4 (для сервера): «Прослушивание» привязанного порта
     *                       для идентификации подключений 
     */
    int error_4_listening = listen(ServSock,
                                   SOMAXCONN);

    if (error_4_listening != 0)
    {
        std::cout << "Can`t start to listen to. Error #"
                  << WSAGetLastError() << std::endl;

        closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета
        WSACleanup();           // деинициализировать сокет
        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Listening..." << std::endl;
    }


    // Этап 5 (только для Сервера). Подтверждение подключения
    sockaddr_in clientInfo;

    ZeroMemory(&clientInfo, sizeof(clientInfo));

    int clientInfo_size = sizeof(clientInfo);

    SOCKET ClientConn = accept(ServSock,
                               reinterpret_cast<sockaddr*>(&clientInfo),
                               &clientInfo_size);

    if (ClientConn == INVALID_SOCKET)
    {
        std::cout << "Client detected, but can`t connect to a client. Error #"
                  << WSAGetLastError() << std::endl;

        closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета сервера
        closesocket(ClientConn);// закрыть дескриптор использовавшегося сокета клиента
        WSACleanup();           // деинициализировать сокеты
        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Connection to a client established successfully" << std::endl;
    }


    // Этап 6: Передача данных между Клиентом и Сервером
    std::vector<char> servBuffer(BUFFER_SIZE);
    std::vector<char> clientBuffer(BUFFER_SIZE);
    short packet_size = 0;


    // Процесс непрерывного перехода от send() к recv() и обратно
    // реализуется через бесконечный цикл,
    // из которого совершается выход по вводу особой комбинации клавиш.
    while (true)
    {
        // приём информации через recv()
        packet_size = recv(ClientConn,
                           servBuffer.data(),
                           servBuffer.size(),
                           0);

        std::cout << "Client`s message: " << servBuffer.data() << std::endl;

        std::cout << "Your (host) message: ";
        fgets(clientBuffer.data(),
              clientBuffer.size(),
              stdin);


        // Check whether server would like to stop chatting
        if (clientBuffer[0] == 'x' &&
            clientBuffer[1] == 'x' &&
            clientBuffer[2] == 'x')
        {
            shutdown(ClientConn, SD_BOTH);
            closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета сервера
            closesocket(ClientConn);// закрыть дескриптор использовавшегося сокета клиента
            WSACleanup();           // деинициализировать сокеты
            return 0;               // возврат caller-у
        }

        // отправка информации через send()
        packet_size = send(ClientConn,
                           clientBuffer.data(),
                           clientBuffer.size(),
                           0);

        if (packet_size == SOCKET_ERROR)
        {
            std::cout << "Can`t send message to Client. Error #"
                      << WSAGetLastError() << std::endl;

            closesocket(ServSock);  // закрыть дескриптор использовавшегося сокета сервера
            closesocket(ClientConn);// закрыть дескриптор использовавшегося сокета клиента
            WSACleanup();           // деинициализировать сокеты
            return 1;               // возврат ошибки caller-у
        }
    }   // end infinity while
}   // end main()
