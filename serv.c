// Local testing server for windows.
// Build with gcc: gcc -o serv.exe serv.c -lws2_32
// Note that this server will only look at the first 1024 bytes of any requests (so a very long url may be cut off)

#include "winsock2.h"
#include "ws2tcpip.h"
#include "stdio.h"

WSADATA wsaData;
int res;
ADDRINFOA hints = (ADDRINFOA){
	.ai_family = AF_INET,
	.ai_socktype = SOCK_STREAM,
	.ai_protocol = IPPROTO_TCP,
	.ai_flags = AI_PASSIVE
};
ADDRINFOA* result = NULL;
SOCKET servsock = INVALID_SOCKET;
SOCKET clientsock = INVALID_SOCKET;
#define REQUEST_MAX_LEN 1023
char request[REQUEST_MAX_LEN+1];
char url[REQUEST_MAX_LEN+1] = ".";

char fileheader[] = "HTTP/1.1 200 OK\r\n\r\n";
char err404[] = "HTTP/1.1 404 Not Found\r\n\r\nthing not found D:\r\n";
char err405[] = "HTTP/1.1 405 Method Not Allowed\r\n\r\nonly http GET is supported D:\r\n";
char err500[] = "HTTP/1.1 500 Server Error\r\n\r\nserver err D:\r\n" ;
char indexdothtml[] = "index.html";

#define cleanExit(code) WSACleanup(); return code;

#define cleanFail(str, code) printf("Failed to " str " (%d)\n",code); cleanExit(code); 

#define cleanFailW(str) cleanFail(str, WSAGetLastError())

#define killClient(client) if (shutdown(client, SD_SEND) == SOCKET_ERROR) closesocket(client)


int main(int argc, char** argv) {

	int urlstart = 0;
	for (urlstart;urlstart<REQUEST_MAX_LEN;urlstart++) {
		if (url[urlstart] == '\0') break;
	}; //Move urlptr to end of local path string

	if (res = WSAStartup(MAKEWORD(1,1), &wsaData)) {
		cleanFail("init winsock", res);
	}
	if (res = getaddrinfo(NULL, "3000", &hints, &result)) {
		cleanFail("getaddrinfo", res);
	}
	servsock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (servsock == INVALID_SOCKET) {
		freeaddrinfo(result);
		cleanFailW("socket");
	}
	res = bind(servsock, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);
	if (res == SOCKET_ERROR) {
		cleanFailW("bind");
	}
	if (listen(servsock, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(servsock);
		cleanFailW("listen");
	}

	while(1) {
		clientsock = accept(servsock, NULL, NULL);
		if (clientsock == INVALID_SOCKET) {
			closesocket(servsock);
			cleanFailW("accept");
		}
		res = recv(clientsock, request, REQUEST_MAX_LEN,0);

		if (request[0] != 'G') { // If not get request
			send(clientsock, err405, sizeof(err405)-1,0);
			killClient(clientsock);
			continue;
		}


		char* reqptr = request + 4; // Skip G E T ` `
		//printf("%s", reqptr);
		int i = urlstart;
		for (i;i<REQUEST_MAX_LEN;i++) { // grab path
			url[i] = *reqptr++;
			if (url[i] == ' ') {
				url[i] = '\0';
				break;
			}
		}
		if (url[i-1] == '/') { // for directories, append index.html (ie /dir/index.html)
			int ctr = 0;
			for (i;i<REQUEST_MAX_LEN;i++) {
				url[i] = indexdothtml[ctr++];
				if (ctr > sizeof(indexdothtml)) break;
			}
		}

		printf("GET %s\n", &url);

		HANDLE file = CreateFileA(url, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
		if (file == INVALID_HANDLE_VALUE) {
			res = GetLastError();
			CloseHandle(file);
			if (res == ERROR_FILE_NOT_FOUND) {
				send(clientsock, err404, sizeof(err404)-1,0);
			} else {
				send(clientsock, err500, sizeof(err500)-1,0);
			}
			killClient(clientsock);
			continue;
		}
		int size = GetFileSize(file, NULL) + sizeof(fileheader) -1;
		printf("%d\n", size);
		char* content = malloc(size);
		memcpy(content, fileheader, sizeof(fileheader)-1);

		int totalBytesRead = sizeof(fileheader) -1 ;
		long unsigned int bytesRead = 0; //dummy value
		while(totalBytesRead < size) {
			if (!ReadFile(file, content+sizeof(fileheader)-1, size - totalBytesRead, &bytesRead, NULL)) {
				CloseHandle(file);
				send(clientsock, err500, sizeof(err500)-1,0);
				killClient(clientsock);
				continue;
			}
			totalBytesRead += bytesRead;
		}
		CloseHandle(file);
		send(clientsock, content, size, 0);
		free(content);
		killClient(clientsock);
	}

	closesocket(servsock);
	cleanExit(0);
}

