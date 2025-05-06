
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "messagerie.h"



Message messages[MAX_MESSAGES];
int message_count = 0;

void send_message(const char *sender, const char *receiver, const char *text) {
    if (message_count >= MAX_MESSAGES) {
        fprintf(stderr, "Boîte de réception pleine.\n");
        return;
    }

    strncpy(messages[message_count].sender, sender, 50);
    strncpy(messages[message_count].receiver, receiver, 50);
    strncpy(messages[message_count].text, text, MAX_TEXT_LEN);
    message_count++;
}

void print_conversation(const char *user1, const char *user2) {
    printf("Conversation entre %s et %s :\n", user1, user2);
    for (int i = 0; i < message_count; i++) {
        if ((strcmp(messages[i].sender, user1) == 0 && strcmp(messages[i].receiver, user2) == 0) ||
            (strcmp(messages[i].sender, user2) == 0 && strcmp(messages[i].receiver, user1) == 0)) {
            printf("%s : %s\n", messages[i].sender, messages[i].text);
        }
    }
}
