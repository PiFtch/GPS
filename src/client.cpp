#define WIN32_LEAN_AND_MEAN
#include "../include/common.h"
// #include "../include/huffman.h"
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


#define DEFAULT_BUFLEN 4096
#define DEFAULT_PORT "27015"
#define MAXCLIENT 10
char *SERVER_NAME = "piftch-laptop";

extern void get_gps_signal(char *buf, gps_signal& sig);
extern void gen_message(char *buf, gps_signal *signal);
extern int compressedSizeUlong(char *srcbuf, int len);
extern void encode(unsigned long *dstbuf);
// memory
gps_signal sig[MAXCLIENT];
HANDLE sig_mutex[MAXCLIENT];
bool flag[MAXCLIENT] = {false};
HANDLE flag_mutex[MAXCLIENT] = {NULL};

bool used[MAXCLIENT] = {false};     // max number of client thread
HANDLE used_mutex = NULL;

void makeSocket(SOCKET& ConnectSocket, char *server_name) {
    WSADATA wsaData;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(1);
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(server_name, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(1);
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(1);
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
}

int Client(int argc, char **argv, int index) {
    SOCKET ConnectSocket = INVALID_SOCKET;
    makeSocket(ConnectSocket, SERVER_NAME);

    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    char compressed[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    while (true) {
        WaitForSingleObject(flag_mutex[index], INFINITE);
        if (!flag[index]) {
            get_gps_signal(sendbuf, sig[index]);
        } else {
            // flag 为true，不随机生成而是直接发送
            gen_message(sendbuf, &sig[index]);
            flag[index] = false;
        }
        ReleaseMutex(flag_mutex[index]);

        int length = compressedSizeUlong(sendbuf, strlen(sendbuf));
        unsigned long *compressed = new unsigned long[length];
        encode(compressed);
        string temp("");
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < 4; j++) {
                sprintf(sendbuf + strlen(sendbuf), "%c", ((char *)compressed) + i * 4 + j);
            }
        }

        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;    
        }

        this_thread::sleep_for(chrono::milliseconds(1000));

        // try to receive data from server
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            // process received data
            int op = atoi(recvbuf);
            // cout << "receive: " << op << endl;
            switch (op)
            {
                case 0:
                    // CONTINUE
                    break;
                case 1:
                    // Quit
                    closesocket(ConnectSocket);
                    WSACleanup();

                    WaitForSingleObject(used_mutex, INFINITE);
                    used[index] = false;
                    ReleaseMutex(used_mutex);
                    return 0;
                    break;
                default:
                    break;
            }
        }
    }
    
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    WaitForSingleObject(used_mutex, INFINITE);
    used[index] = false;
    ReleaseMutex(used_mutex);
}

int main(int argc, char **argv) 
{
    // Initialize mutex handles
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
                int hour, minute, second, millisecond;
                int hemi, d, m, s;
                int hemi1, d1, m1, s1;
                cout << "enter hour minute seond millisecond" << endl;
                cin >> hour >> minute >> second >> millisecond;
                cout << "enter latitude: degree minute second hemisphere(0/1)" << endl;
                cin >> d >> m >> s >> hemi;
                cout << "enter longitude: degree minute second hemisphere(0/1)" << endl;
                cin >> d1 >> m1 >> s1 >> hemi1;
                WaitForSingleObject(flag_mutex[index], INFINITE);
                flag[index] = true;
                sig[index].t = new Time(hour, minute, second, millisecond);
                sig[index].latitude = new Latitude(hemi, d, m, s);
                sig[index].longitude = new Longitude(hemi1, d1, m1, s1);
                ReleaseMutex(flag_mutex[index]);
            }
        }
    }
}