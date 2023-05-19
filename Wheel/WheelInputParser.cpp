#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <wchar.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "hidapi.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

DWORD WINAPI clientHandler(LPVOID lpParam);

hid_device* handle;

int main(void)
{
    //Hid reader
    int res;
    unsigned char buf[256];
#define MAX_STR 255
    wchar_t wstr[MAX_STR];
    
    int i;
    struct hid_device_info* devs, * cur_dev;

    if (hid_init())
        return -1;

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    bool flag = false;

    while (cur_dev) {
        if (cur_dev->vendor_id == 0x044f && cur_dev->product_id == 0xb697) {
            printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
            printf("\n");
            printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
            printf("  Product:      %ls\n", cur_dev->product_string);
            printf("  Release:      %hx\n", cur_dev->release_number);
            printf("  Interface:    %d\n", cur_dev->interface_number);
            printf("\n");
            flag = true;
        }
        cur_dev = cur_dev->next;
    }

    if (!flag) {
        printf("Device not Found!\n");
        return 0;
    }

    hid_free_enumeration(devs);

    // Set up the command buffer.
    memset(buf, 0x00, sizeof(buf));
    buf[0] = 0x01;
    buf[1] = 0x81;


    // Open the device using the VID, PID,
    // and optionally the Serial number.
    ////handle = hid_open(0x4d8, 0x3f, L"12345");
    handle = hid_open(0x044f, 0xb697, NULL);
    if (!handle) {
        printf("unable to open device\n");
        return 1;
    }

    // Read the Manufacturer String
    wstr[0] = 0x0000;
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read manufacturer string\n");
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    wstr[0] = 0x0000;
    res = hid_get_product_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read product string\n");
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read serial number string\n");
    printf("Serial Number String: (%d) %ls", wstr[0], wstr);
    printf("\n");

    // Read Indexed String 1
    wstr[0] = 0x0000;
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read indexed string 1\n");
    printf("Indexed String 1: %ls\n", wstr);

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(handle, 1);

    // Try to read from the device. There shoud be no
    // data here, but execution should not block.
    res = hid_read(handle, buf, 17);

    // Send a Feature Report to the device
    buf[0] = 0x2;
    buf[1] = 0xa0;
    buf[2] = 0x0a;
    buf[3] = 0x00;
    buf[4] = 0x00;
    res = hid_send_feature_report(handle, buf, 17);
    if (res < 0) {
        printf("Unable to send a feature report.\n");
    }

    memset(buf, 0, sizeof(buf));

    // Read a Feature Report from the device
    buf[0] = 0x2;
    res = hid_get_feature_report(handle, buf, sizeof(buf));
    if (res < 0) {
        printf("Unable to get a feature report.\n");
        printf("%ls", hid_error(handle));
    }
    else {
        // Print out the returned buffer.
        printf("Feature Report\n   ");
        for (i = 0; i < res; i++)
            printf("%02hhx ", buf[i]);
        printf("\n");
    }

    memset(buf, 0, sizeof(buf));

    //WinSock2 Server
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
    unsigned char read[64];

    while (1) {
        int res = hid_read(handle, read, sizeof(read));
        sprintf_s(sendbuf, DEFAULT_BUFLEN, "%03d,%03d,%03d\n", (int)read[2], read[4], read[6]);


        int iSendResult = send(ClientSocket, sendbuf, strlen(sendbuf), 0);

        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            break;
        }

        Sleep(100);

        if (_kbhit())
            break;
    }

    // shutdown the connection since we're done
    int iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }

    // cleanup
    closesocket(ClientSocket);
    return 0;
}