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

const char* szResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
const char szHtml[] = R"(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Server Response From main.cpp</title>
  </head>
  <body>
    <h1>This HTML content is the server response from main.cpp</h1>
    <button id="click-btn"></button>
  </body>
  <script>
    var count = 0;
    function handleClick() {
      count += 1;
      let btn = document.getElementById("click-btn");
      btn.innerHTML = count.toString();
    }
    window.onload = function() {
      let btn = document.getElementById("click-btn");
      btn.innerHTML = count.toString();
      btn.addEventListener("click", handleClick);
    };
  </script>
</html>
)";

std::string readFile(const std::string& fname) {
  std::ifstream file(fname);
  if (!file.is_open()) {
	printf("main.cpp :: Could not open / read '%s'\n", fname);
	return "";
  }
  std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  return content;
}

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
  printf("main.cpp :: Server started on port %d\n", PORT);
  fflush(stdout);
  while (true) {
    SOCKADDR_IN clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
      continue;
    }
    printf("main.cpp :: Client connected to server\n");
    fflush(stdout);
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived == SOCKET_ERROR) {
      printf("main.cpp :: Error occured while receiving data from client\n");
      continue;
    }
    std::string httpRequest(buffer, bytesReceived);
    if (httpRequest.find("GET / HTTP") != std::string::npos) {
      send(clientSocket, szResponse, strlen(szResponse), 0);
      send(clientSocket, szHtml, strlen(szHtml), 0);
    } else {
      send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
    }
    printf("main.cpp :: Server has sent HTML response to client GET request\n");
    fflush(stdout);
    closesocket(clientSocket);
    /*send(clientSocket, szResponse, strlen(szResponse), 0);
    const char* p = szHtml;
    int remaining = strlen(szHtml);
    while (remaining > 0) {
      int sentBytes = send(clientSocket, p, remaining, 0);
      if (sentBytes <= 0) {
	if (sentBytes == 0) {
          printf("main.cpp :: Client disconnected from server unexpectedly\n");
        } else {
	  printf("main.cpp :: Error occured while sending data to client\n");
        }
	fflush(stdout);
	break;
      }
      p += sentBytes;
      remaining -= sentBytes;
    }
    printf("main.cpp :: Server has responded to client socket GET request with HTML content\n");
    fflush(stdout);*/
  }
  closesocket(server);
  WSACleanup();
  return 0;
}
