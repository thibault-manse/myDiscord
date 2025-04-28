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
char* client_names[MAX_CLIENTS];  // Tableau pour stocker les noms des clients
int client_count = 0;
CRITICAL_SECTION cs;  // Pour protéger les accès partagés

// Fonction pour nettoyer le nom d'utilisateur (supprimer les retours à la ligne, espaces)
void clean_username(char* username) {
    username[strcspn(username, "\n")] = 0;  // Enlever le retour à la ligne
}

DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    int len;
    char client_name[100];

    // Demander le nom de l'utilisateur
    send(client_socket, "Entrez votre nom d'utilisateur: ", 32, 0);
    len = recv(client_socket, client_name, sizeof(client_name), 0);
    client_name[len] = '\0';
    clean_username(client_name);

    // Enregistrer le nom de l'utilisateur
    EnterCriticalSection(&cs);
    client_names[client_count] = strdup(client_name);  // Enregistrer le nom
    clients[client_count++] = client_socket;
    LeaveCriticalSection(&cs);

    printf("Utilisateur %s connecté.\n", client_name);

    // Main loop pour recevoir et envoyer des messages
    while ((len = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[len] = '\0';

        // Si c'est un message privé
        if (strncmp(buffer, "/privateto ", 11) == 0) {
            char* recipient_name = buffer + 11;
            char* message = strchr(recipient_name, ' ');
            if (message != NULL) {
                *message = '\0';  // Séparer le nom du message
                message++;  // Ignorer l'espace
                int recipient_index = -1;

                // Chercher le destinataire parmi les clients
                EnterCriticalSection(&cs);
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(client_names[i], recipient_name) == 0) {
                        recipient_index = i;
                        break;
                    }
                }

                // Si le destinataire est trouvé, envoyer le message
                if (recipient_index != -1) {
                    send(clients[recipient_index], message, strlen(message), 0);
                    printf("Message privé envoyé à %s: %s\n", recipient_name, message);
                } else {
                    char* error_msg = "Utilisateur introuvable.\n";
                    send(client_socket, error_msg, strlen(error_msg), 0);
                }
                LeaveCriticalSection(&cs);
            }
        } else {
            // Si c'est un message général (broadcast)
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
                client_names[j] = client_names[j + 1];
            }
            client_count--;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    closesocket(client_socket);
    free(client_socket_ptr);
    printf("Utilisateur déconnecté.\n");

    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);

    InitializeCriticalSection(&cs);

    // Initialisation de WinSock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Création de la socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Lier la socket au port
    bind(server_socket, (struct sockaddr *)&server, sizeof(server));

    // Écouter les connexions
    listen(server_socket, MAX_CLIENTS);
    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        // Accepter une connexion cliente
        client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len);
        printf("Nouveau client connecté.\n");

        // Si le serveur est plein, refuser la connexion
        EnterCriticalSection(&cs);
        if (client_count < MAX_CLIENTS) {
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

    // Fermeture de la socket serveur
    closesocket(server_socket);
    DeleteCriticalSection(&cs);
    WSACleanup();

    return 0;
}
