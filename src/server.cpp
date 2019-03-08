#include <WinSock2.h>
#include <Windows.h>
#include "../include/common.h"
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 4096

/*****MESSAGE*****/
#define CONTINUE_TRANSIMISSION 0
#define CLOSE_CONNECTION 1

char *command[3] = {"0",
                    "1"};
/*****************/

#define MAX_CONNECTION 20

extern Node* resolve(char *buf);
Node *First[MAX_CONNECTION];

bool used[MAX_CONNECTION] = {false};
HANDLE used_mutex = NULL;

char message[MAX_CONNECTION] = {'0'};
HANDLE message_mutex[MAX_CONNECTION];

thread server_thread[10];
SOCKET ClientSockets[MAX_CONNECTION];

int handle(SOCKET ClientSocket, int index) {
    Node *first = new Node();
    Node *p = first;
    Node *current;

    sockaddr_in temp;
    int len = sizeof(temp);
    getsockname(ClientSocket, (sockaddr *)&temp, &len);
    // cout << temp.sin_family << ' ' << temp.sin_addr.S_un.S_addr << ':' << temp.sin_port << endl;
    char filename[100] = "log/server/";
    
    char tempaddr[40];
    strcat(filename, itoa(temp.sin_addr.S_un.S_addr, tempaddr, 10));
    strcat(filename, "-");
    
    strcat(filename, itoa(temp.sin_port, tempaddr, 10));
    cout << "filename: " << filename << endl;
    fstream file, fileResolve;
    file.open(filename, fstream::out);
    strcat(filename, " Resolve");
    fileResolve.open(filename, fstream::out);
    file << "family: " << temp.sin_family << " address: " << temp.sin_addr.S_un.S_addr << ':' << temp.sin_port << endl;
    
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    char sendbuf[DEFAULT_BUFLEN] = "";
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            // printf("Bytes received: %d\n", iResult);
            file << "Bytes received: " << iResult << endl;
            file << recvbuf << endl;
            // Echo the command to the sender
            // strcat(sendbuf, &message[index]);
            current = resolve(recvbuf);
            p->next = current;
            p = current;
            fileResolve << current->t.hour << ':' << current->t.minute << ':' << current->t.second << ':' << current->t.millisecond << ','
                        << current->latitude.degree << "°" << current->latitude.minute << "'" << current->latitude.second << "\"" << current->latitude.hemisphere << ','
                        << current->longitude.degree << "°" << current->longitude.minute << "'" << current->longitude.second << "\"" << current->longitude.hemisphere << endl;

            iSendResult = send(ClientSocket, &message[index], 1, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                WaitForSingleObject(used_mutex, INFINITE);
                used[index] = false;
                ReleaseMutex(used_mutex);
                return 1;
            }
            // printf("Bytes sent: %d\n", iSendResult);
            file << "Message sent: " << message[index] << endl;
            message[index] = '0';
            // file << "Bytes sent: " << iSendResult << endl;

        } else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    file.close();
    fileResolve.close();
    First[index] = first;
    closesocket(ClientSocket);
    WaitForSingleObject(used_mutex, INFINITE);
    used[index] = false;
    ReleaseMutex(used_mutex);
}

void makeSocket(SOCKET& ListenSocket) {
    WSADATA wsaData;
    int iResult;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "failed" << endl;
        exit(1);
    }

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo: failed" << endl;
        WSACleanup();
        exit(1);
    }

    ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "socket():error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }
    freeaddrinfo(result);

}

void monitor() {
    char op;
    while (true) {
        cout << "GPS Server Monitor." << endl;
        cout << "Options:" << endl;
        cout << "   r -- show(refresh) connection slots." << endl;
        cout << "   m -- choose a connection to send a command to the client." << endl;
        cin >> op;
        if (op == 'r') {
            cout << "Connection slots:" << endl;
            for (int i = 0; i < MAX_CONNECTION; i++) {
                printf("% 3d", i);
            }
            cout << endl;
            WaitForSingleObject(used_mutex, INFINITE);
            for (int i = 0; i < MAX_CONNECTION; i++) {
                if (used[i]) {
                    printf("% 3c", '+');
                } else {
                    printf("% 3c", '-');
                }
            }
            ReleaseMutex(used_mutex);
            cout << endl;    
        } else if (op == 'm') {
            cout << "Select a client to send command." << endl;
            for (int i = 0; i < MAX_CONNECTION; i++) {
                printf("%3d", i);
            }
            cout << endl;
            WaitForSingleObject(used_mutex, INFINITE);
            for (int i = 0; i < MAX_CONNECTION; i++) {
                if (used[i]) {
                    printf("%03c", '+');
                } else {
                    printf("%03c", '-');
                }
            }
            ReleaseMutex(used_mutex);
            cout << endl;    
            int index;
            cin >> index;
            if (index < 0 || index >= MAX_CONNECTION) {
                cout << "Invalid index!" << endl;
            } else if (!used[index]) {
                cout << "This connection slot has not been used!" << endl;
            } else {
                // modify message to be sent
                cout << "0 to continue transmittion, 1 to close connection." << endl;
                WaitForSingleObject(message_mutex[index], INFINITE);
                cin >> message[index];
                ReleaseMutex(message_mutex[index]);
            }
        }
    }

}

int main(int argc, char *argv[]) {
    used_mutex = CreateMutex(NULL, false, "used_mutex");
    for (int i = 0; i < MAX_CONNECTION; i++) {
        message_mutex[i] = CreateMutex(NULL, false, "message_mutex");
    }
    SOCKET ListenSocket;
    makeSocket(ListenSocket);
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    
    thread monitor_thread(monitor);
    monitor_thread.detach();

    while (true) {
        SOCKET tempSOCKET = accept(ListenSocket, NULL, NULL);
        if (tempSOCKET == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        // firstly check if there exists available slot
        int index;
        WaitForSingleObject(used_mutex, INFINITE);
        for (index = 0; index < MAX_CONNECTION; index++) {
            if (!used[index]) {
                used[index] = true;    
                break;       
            }
        }
        ReleaseMutex(used_mutex);
        if (index >= MAX_CONNECTION) {
            cout << "no available slot." << endl;
            continue;
        }
        thread t(handle, tempSOCKET, index);
        t.detach();
    }
    WSACleanup();
    return 0;
}