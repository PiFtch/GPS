#define WIN32_LEAN_AND_MEAN
#include "../include/common.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define MAXCLIENT 10
const char *SERVER_NAME = "piftch-laptop";

extern void get_gps_signal(char *buf, gps_signal& sig);
extern void gen_message(char *buf, gps_signal *signal);

// memory
gps_signal sig[MAXCLIENT];
bool flag[MAXCLIENT] = {false};
HANDLE flag_mutex[MAXCLIENT] = {NULL};

bool used[MAXCLIENT] = {false};     // max number of client thread
HANDLE used_mutex = NULL;

int Client(int argc, char **argv, int index) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    // char *sendbuf = "this is a test";
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];

    WaitForSingleObject(flag_mutex[index], INFINITE);
    if (!flag[index]) {
        get_gps_signal(sendbuf, sig[index]);
    } else {
        // flag 为true，不随机生成而是直接发送
        gen_message(sendbuf, &sig[index]);
        flag[index] = false;
    }
    ReleaseMutex(flag_mutex[index]);

    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    for (int i = 1; i <= 10; i++) {
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    
    // cout << "Thread id: " << this_thread::get_id() << ' ';
    this_thread::sleep_for(chrono::microseconds(100));
    // printf("Bytes Sent: %ld\n", iResult);
    this_thread::sleep_for(chrono::milliseconds(2000));
    }

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) ;
            // printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 );
            // printf("Connection closed\n");
        else ;
            // printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    WaitForSingleObject(used_mutex, INFINITE);
    used[index] = false;
    ReleaseMutex(used_mutex);
}

int main(int argc, char **argv) 
{
    // sem = CreateSemaphore()
    used_mutex = CreateMutex(NULL, false, "used_mutex");
    for (int i = 0; i < MAXCLIENT; i++) {
        flag_mutex[i] = CreateMutex(NULL, false, "flag_mutex");
    }
    // Main thread
    char op;
    cout << "GPS Client." << endl;
    cout << "Options:" << endl;
    cout << "1 to create a client thread sending GPS signals." << endl;
    cout << "2 to select a client thread and modify its data." << endl;
    cout << "Press any other key to quit." << endl;
    while (true) {
        cin >> op;
        if (op == '1') {
            int index;
            WaitForSingleObject(used_mutex, INFINITE);
            for (index = 0; index < MAXCLIENT; index++) {
                if (!used[index]) {// find available slot
                    used[index] = true;
                    break;
                }
            }
            ReleaseMutex(used_mutex);
            if (index >= MAXCLIENT) {
                cout << "No available thread slot, please try again." << endl;
            } else {
                cout << "Available thread slot index " << index << endl;
                thread thread_client(Client, argc, argv, index);
                thread_client.detach();
            }
        } else if (op == '2') {
            cout << "There could be " << MAXCLIENT << " client thread at most, choose a thread to modify the data to be sent." << endl;
            for (int i = 0; i < MAXCLIENT; i++) {
                printf("%3d", i);
            }
            cout << endl;
            for (int i = 0; i < MAXCLIENT; i++) {
                if (used[i])
                    printf("%3c", '+');
                else
                    printf("%3c", '-');
            }
            cout << endl;
            int index;
            cin >> index;
            if (index < 0 || index >= MAXCLIENT) {
                cout << "Invalid index: " << index << endl;
                continue;
            } else if (!used[index]) {
                cout << "This thread slot has not been used!" << endl;
                continue;
            } else {
                // start modify

            }
        }
    }
/*
    if (op == 1) {
        thread thread_client(Client, argc, argv);
        thread thread2(Client, argc, argv);
        thread_client.join();
        
        thread2.join();
    }
*/
}