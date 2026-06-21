// Network_1__client.cpp: - клиентская часть (TCP)

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

int main()                  // CLIENT
{
    const char SERVER_IP[] = "";    // Enter IPv4 address of Server
    const int SERVER_PORT_NUM = 0;  // Enter Listening port on Server side

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
    SOCKET ClientSock = socket(AF_INET,
                               SOCK_STREAM,
                               0);

    if (ClientSock == INVALID_SOCKET)
    {
        std::cout << "Error initialization Client socket #"
                  << WSAGetLastError() << std::endl;

        closesocket(ClientSock);// закрыть дескриптор использовавшегося сокета
        WSACleanup();           // деинициализировать сокет
        return 1;               // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Client socket initialization is OK" << std::endl;
    }


    /* 3. «Привязка» созданного сокета к конкретной паре IP-адрес/Порт -
     *    с этого момента данный сокет(его имя) будет ассоциироваться
     *    с конкретным процессом, который «висит» по указанному адресу и порту.
     */

    in_addr ip_to_num;

    // функция inet_pton(), которая переводит обычную строку типа char[],
    // содержащую IPv4 адрес в привычном виде с точками-разделителями
    // в структуру типа in_addr
    int error_2_IP_translation = inet_pton(AF_INET,     // int <семейство адресов>
                                           SERVER_IP,
                                           &ip_to_num);

    if (error_2_IP_translation <= 0)
    {
        std::cout << "Error in IP translation to special numeric format" << std::endl;
        return 1;               // возврат ошибки caller-у
    }

    // Establishing a connection to Server
    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr   = ip_to_num;    // Server's IPv4 after inet_pton() function
    servInfo.sin_port   = htons(SERVER_PORT_NUM);


    /* Этап 4 (для клиента): Для клиентской части приложения:
     *                       запуск процедуры подключения к серверному сокету
     *                       (должны знать его IP-адрес/Порт).
     */
    int error_3_connect = connect(ClientSock,
                                  (sockaddr*)&servInfo,
                                  sizeof(servInfo)
    );

    if (error_3_connect != 0)
    {
        std::cout << "Connection to server is FAILED. Error #"
                  << WSAGetLastError() << std::endl;

        closesocket(ClientSock);    // закрыть дескриптор использовавшегося сокета
        WSACleanup();               // деинициализировать сокет
        return 1;                   // возврат ошибки caller-у
    }
    else
    {
        std::cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << std::endl;
    }


    // Этап 6: Передача данных между Клиентом и Сервером
    std::vector<char> servBuffer(BUFFER_SIZE);
    std::vector<char> clientBuffer(BUFFER_SIZE);
    short packet_size = 0;  // The size of sending / receiving packet in bytes


    // Процесс непрерывного перехода от send() к recv() и обратно
    // реализуется через бесконечный цикл,
    // из которого совершается выход по вводу особой комбинации клавиш.
    while (true)
    {
        std::cout << "Your (Client) message to Server: ";
        // fgets() - чтение всей строки до нажатия кнопки Ввода
        fgets(clientBuffer.data(),
              clientBuffer.size(),
              stdin
        );

        // Check whether client would like to stop chatting
        if (clientBuffer[0] == 'x' &&
            clientBuffer[1] == 'x' &&
            clientBuffer[2] == 'x')
        {
            shutdown(ClientSock, SD_BOTH);
            closesocket(ClientSock);    // закрыть дескриптор использовавшегося сокета
            WSACleanup();               // деинициализировать сокет
            return 0;                   // возврат caller-у
        }

        // отправка информации через send()
        packet_size = send(ClientSock,
                           clientBuffer.data(),
                           clientBuffer.size(),
                           0
        );

        if (packet_size == SOCKET_ERROR)
        {
            std::cout << "Can`t send message to Server. Error #"
                      << WSAGetLastError() << std::endl;

            closesocket(ClientSock);// закрыть дескриптор использовавшегося сокета
            WSACleanup();           // деинициализировать сокет
            return 1;               // возврат ошибки caller-у
        }


        // приём информации через recv()
        packet_size = recv(ClientSock,
                           servBuffer.data(),
                           servBuffer.size(),
                           0
        );

        if (packet_size == SOCKET_ERROR) {
            std::cout << "Can't receive message from Server. Error # "
                      << WSAGetLastError() << std::endl;

            closesocket(ClientSock);// закрыть дескриптор использовавшегося сокета
            WSACleanup();           // деинициализировать сокет
            return 1;               // возврат ошибки caller-у
        }
        else
            std::cout << "Server message: " << servBuffer.data() << std::endl;
    }   // end infinity while

    closesocket(ClientSock);        // закрыть дескриптор использовавшегося сокета
    WSACleanup();                   // деинициализировать сокет

    return 0;
}   // end main()
