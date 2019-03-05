#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

thread server_thread[10];
SOCKET ClientSockets[10];

int handle(SOCKET ClientSocket, int i) {
    char filename[10];
    itoa(i, filename, 10);
    fstream file;
    file.open(filename, fstream::out);
    // cout << "Thread id: " << this_thread::get_id() << endl;
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            file << "Bytes received: " << iResult << endl;

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);

            file << "Bytes sent: " << iSendResult << endl;

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
    closesocket(ClientSocket);
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

int main(int argc, char *argv[]) {
    SOCKET ListenSocket;
    makeSocket(ListenSocket);
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    for (int i = 0; i < 10; i++) {
        if ((ClientSockets[i] = accept(ListenSocket, NULL, NULL)) == INVALID_SOCKET){
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        thread t(handle, ClientSockets[i], i);
        t.detach();
    }
    WSACleanup();
    return 0;
}