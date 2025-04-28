#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h> // Ajouté pour utiliser time() et localtime()

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

SOCKET sock;
volatile int is_running = 1;

void set_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void print_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now); // Utilisation correcte de localtime
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

DWORD WINAPI receive_messages(LPVOID arg) {
    char buffer[BUFFER_SIZE];
    int len;

    while (is_running) {
        len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (len > 0) {
            buffer[len] = '\0';
            set_color(11);
            print_time();
            printf("Message : %s", buffer);
            set_color(7);
        } else {
            set_color(12);
            printf("\nDéconnecté du serveur.\n");
            set_color(7);
            is_running = 0;
            break;
        }
    }
    return 0;
}

DWORD WINAPI send_messages(LPVOID arg) {
    char message[BUFFER_SIZE];

    while (is_running) {
        printf("> ");
        fgets(message, BUFFER_SIZE, stdin);

        if (strncmp(message, "/quit", 5) == 0) {
            is_running = 0;
            break;
        }

        if (send(sock, message, strlen(message), 0) < 0) {
            set_color(12);
            printf("Erreur d'envoi\n");
            set_color(7);
            break;
        }
    }
    return 0;
}

int main() {
    WSADATA wsa;
    struct sockaddr_in server;

    printf("=== Client Chat en C ===\n");

    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connexion échouée.\n");
        return 1;
    }

    set_color(10);
    printf("Connecté à %s:%d\n", SERVER_IP, PORT);
    set_color(7);

    CreateThread(NULL, 0, receive_messages, NULL, 0, NULL);
    CreateThread(NULL, 0, send_messages, NULL, 0, NULL);

    while (is_running) Sleep(100); // Attente passive

    closesocket(sock);
    WSACleanup();

    set_color(14);
    printf("Session terminée. À bientôt !\n");
    set_color(7);

    return 0;
}
