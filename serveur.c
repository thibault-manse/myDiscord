// serveur.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

SOCKET clients[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION cs; // Pour protéger les accès partagés

DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[len] = '\0';

        // Broadcast à tous les clients
        EnterCriticalSection(&cs);
        for (int i = 0; i < client_count; i++) {
            if (clients[i] != client_socket) {
                send(clients[i], buffer, strlen(buffer), 0);
            }
        }
        LeaveCriticalSection(&cs);
    }

    // Client déconnecté
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    closesocket(client_socket);
    free(client_socket_ptr);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);

    InitializeCriticalSection(&cs);

    WSAStartup(MAKEWORD(2, 2), &wsa);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    listen(server_socket, MAX_CLIENTS);

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len);
        printf("Nouveau client connecté.\n");

        EnterCriticalSection(&cs);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = client_socket;
            SOCKET *client_ptr = malloc(sizeof(SOCKET));
            *client_ptr = client_socket;
            CreateThread(NULL, 0, handle_client, client_ptr, 0, NULL);
        } else {
            char *msg = "Serveur plein. Connexion refusée.\n";
            send(client_socket, msg, strlen(msg), 0);
            closesocket(client_socket);
        }
        LeaveCriticalSection(&cs);
    }

    closesocket(server_socket);
    DeleteCriticalSection(&cs);
    WSACleanup();
    return 0;
}
