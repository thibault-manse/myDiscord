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
char client_names[MAX_CLIENTS][50];  // Nom pour chaque client
int client_count = 0;
CRITICAL_SECTION cs;

DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[len] = '\0';

        // Vérifier si c'est un message privé
        if (strncmp(buffer, "/private", 8) == 0) {
            char recipient[50];
            sscanf(buffer, "/private %s", recipient);

            // Rechercher l'index du client avec le pseudo
            int found = 0;
            for (int i = 0; i < client_count; i++) {
                if (strcmp(client_names[i], recipient) == 0) {
                    send(clients[i], buffer + strlen("/private ") + strlen(recipient) + 1, strlen(buffer) - strlen("/private ") - strlen(recipient) - 1, 0);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                char *msg = "Utilisateur introuvable.\n";
                send(client_socket, msg, strlen(msg), 0);
            }
        } else {
            // Broadcast à tous les clients
            EnterCriticalSection(&cs);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] != client_socket) {
                    send(clients[i], buffer, strlen(buffer), 0);
                }
            }
            LeaveCriticalSection(&cs);
        }
    }

    // Client déconnecté
    EnterCriticalSection(&cs);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
                strcpy(client_names[j], client_names[j + 1]);  // Deplacer le nom du client aussi
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

            // Demander le pseudo de l'utilisateur
            send(client_socket, "Entrez votre pseudo : ", 22, 0);
            recv(client_socket, client_names[client_count - 1], sizeof(client_names[client_count - 1]), 0);
            printf("Client connecté : %s\n", client_names[client_count - 1]);

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
