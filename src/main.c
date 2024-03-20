#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment("lib", "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <fstream>
#include <string>

#define PORT 8080

const char szResponse[] = "";
const char szPage[] = R"(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Server Response From main.c</title>
  </head>
  <body>
    <h1>This is the response from the server in main.c</h1>
  </body>
</html>
)";

int main(const int argc, const char* argv[]) {
  WSAData wsaData;
  WORD DllVersion MAKEWORD(2, 1);
  if (WSAStartup(DllVersion, &wsaData) != 0) {
    ExitProcess(EXIT_FAILURE);
  }
  SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == INVALID_SOCKET) {
    ExitProcess(EXIT_FAILURE);
  }
  SOCKADDR_IN serverAddr;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_family = AF_INET;
  if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
    ExitProcess(EXIT_FAILURE);
  }
  if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
    ExitProcess(EXIT_FAILURE);
  }
  printf("main.c :: Server started on port %d\n", PORT);
  fflush(stdout);
  while (true) {
    SOCKADDR_IN clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
      continue;
    }
    printf("main.c :: Client connected to server\n");
    fflush(stdout);
    send(clientSocket, szResponse, strlen(szResponse), 0);
    const char* p = szPage;
    int remaining = strlen(szPage);
    while (remaining > 0) {
      int sentBytes = send(clientSocket, p, remaining, 0);
      if (sentBytes <= 0) {
	if (sentBytes == 0) {
          printf("main.c :: Client disconnected from server unexpectedly\n");
        } else {
	  printf("main.c :: Error occured while sending data to client\n");
        }
	fflush(stdout);
	break;
      }
      p += sentBytes;
      remaining -= sentBytes;
    }
    printf("main.c :: Server has responded to client socket GET request with HTML content\n");
    fflush(stdout);
  }
  closesocket(server);
  WSACleanup();
  return 0;
}
