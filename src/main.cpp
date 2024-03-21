#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment("lib", "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstring>

#define PORT 8080

const char* szResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
const char szHtmlIndex[] = R"(
<!DOCTYPE HTML>
<html>
  <head>
    <title>Server Response From main.cpp</title>
  </head>
  <body>
    <h1>This HTML content is the server response from main.cpp</h1>
    <button id="click-btn"></button>
	<button id="nav-btn">Go to other content</button>
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
	  let navbtn = document.getElementById("nav-btn");
	  navbtn.addEventListener("click", function() {
	    window.location.href = "http://localhost:8080/other";
	  });
    };
  </script>
</html>
)";
const char szHtmlOther[] = R"(
<!DOCTYPE HTML>
<hmtl>
  <head>
    <title>Server Response From main.cpp</title>
  </head>
  <body>
    <h1>This is some other content you have been routed to.</h1>
	<button id="nav-btn">Go to the home page</button>
  </body>
  <script>
    window.onload = function() {
	  let btn = document.getElementById("nav-btn");
	  btn.addEventListener("click", function() {
	    window.location.href = "http://localhost:8080/";
	  });
	};
  </script>
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
	printf("main.cpp :: Server has received raw client request\n");
    std::string httpRequest(buffer, bytesReceived);
	printf("main.cpp :: Raw Client HTTP Request:\n%s\n", httpRequest.c_str());
	std::string path;
	size_t start = httpRequest.find("GET ");
    if (start != std::string::npos) {
	  start += 4;
	  size_t end = httpRequest.find(' ', start);
	  if (end != std::string::npos) {
		path = httpRequest.substr(start, end - start);
	  }
	}
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	if (path == "/" || path == "/index.html" || path == "index" || path == "home") {
	  send(clientSocket, szResponse, strlen(szResponse), 0);
	  send(clientSocket, szHtmlIndex, strlen(szHtmlIndex), 0);
	} else if (path == "/other") {
	  send(clientSocket, szResponse, strlen(szResponse), 0);
	  send(clientSocket, szHtmlOther, strlen(szHtmlOther), 0);
	} else {
	  send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
	}
    printf("main.cpp :: Server has sent HTML response to client GET request\n");
    fflush(stdout);
    closesocket(clientSocket);
  }
  closesocket(server);
  WSACleanup();
  return 0;
}
