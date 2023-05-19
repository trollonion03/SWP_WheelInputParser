#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

DWORD WINAPI clientHandler(LPVOID lpParam);

int main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;

    //char recvbuf[DEFAULT_BUFLEN];
    //int recvbuflen = DEFAULT_BUFLEN;

    SYSTEMTIME sys_time;
    char sendbuf[DEFAULT_BUFLEN];

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    while (1)
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // Create a new thread for the incoming connection
        DWORD dwThreadId;
        HANDLE hThread = CreateThread(NULL, 0, clientHandler, (LPVOID)ClientSocket, 0, &dwThreadId);
        if (hThread == NULL)
        {
            printf("CreateThread failed with error: %d\n", GetLastError());
            closesocket(ClientSocket);
        }
        else
        {
            // Detach the thread and let it run on its own
            CloseHandle(hThread);
        }
    }
    // No longer need server socket
    closesocket(ListenSocket);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

DWORD WINAPI clientHandler(LPVOID lpParam)
{
    SOCKET ClientSocket = (SOCKET)lpParam;

    SYSTEMTIME sys_time;
    char sendbuf[DEFAULT_BUFLEN];

    GetLocalTime(&sys_time);
    sprintf_s(sendbuf, DEFAULT_BUFLEN, "%03d,%03d,%03d", 332, 512, 512);

    int iSendResult = send(ClientSocket, sendbuf, strlen(sendbuf), 0);

    if (iSendResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
    }

    // shutdown the connection since we're done
#if 0
    int iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }
#endif

    // cleanup
    //closesocket(ClientSocket);
    return 0;
}