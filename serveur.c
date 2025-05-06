// serveur.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include "messagerie.h"
#include <libpq-fe.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);
    
    // Désactive le buffering TCP pour ce client
    int flag = 1;
    setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    

    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[len] = '\0';
        printf("Message Reçu %d octets : \"%s\"\n", len, buffer);
        fflush(stdout);
    

        // Commande spéciale : GET:expediteur:destinataire
        if (strncmp(buffer, "GET:", 4) == 0) {
            char *user1 = strtok(buffer + 4, ":");
            char *user2 = strtok(NULL, "\0");
            if (user1 && user2) {
                FILE *fp = tmpfile();
                if (!fp) continue;
                int original_stdout = dup(STDOUT_FILENO);
                dup2(fileno(fp), STDOUT_FILENO);

                print_conversation(user1, user2);

                fflush(stdout);
                dup2(original_stdout, STDOUT_FILENO);
                close(original_stdout);

                fseek(fp, 0, SEEK_SET);
                char line[BUFFER_SIZE];
                while (fgets(line, sizeof(line), fp)) {
                    send(client_socket, line, strlen(line), 0);
                }
                fclose(fp);
            }
            continue; // ne pas envoyer aux autres clients
        }
    }
        // Format attendu : sender:receiver:message
char copy[BUFFER_SIZE];
strncpy(copy, buffer, sizeof(copy));
copy[sizeof(copy) - 1] = '\0';  // pour être sûr qu'on n'a pas de dépassement

char *sender = strtok(copy, ":");
char *receiver = strtok(NULL, ":");
char *text = strtok(NULL, "\0");

if (sender && receiver && text) {
    send_message(sender, receiver, text);
    printf("Message reçu : [%s -> %s] %s\n", sender, receiver, text);
    fflush(stdout);

    // Envoi à tous les autres clients
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != client_socket) {
            send(clients[i], buffer, strlen(buffer), 0);  // On utilise buffer intact
        }
    }
    pthread_mutex_unlock(&lock);
}


    // Client déconnecté
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    const char *conninfo = "dbname=mydiscord user=postgres password=dvjmfr5e host=localhost port=5432";

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(server_socket, MAX_CLIENTS);
    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        printf("Nouveau client connecté.\n");

        pthread_mutex_lock(&lock);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = client_socket;

            int *client_ptr = malloc(sizeof(int));
            *client_ptr = client_socket;
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, client_ptr);
            pthread_detach(tid);
        } else {
            char *msg = "Serveur plein. Connexion refusée.\n";
            send(client_socket, msg, strlen(msg), 0);
            close(client_socket);
        }
        pthread_mutex_unlock(&lock);
    }

    close(server_socket);
    pthread_mutex_destroy(&lock);
    return 0;
}
