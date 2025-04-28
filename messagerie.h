
#ifndef MESSAGERIE_H
#define MESSAGERIE_H

#define MAX_MESSAGES 100
#define MAX_TEXT_LEN 1024

typedef struct {
    char sender[50];
    char receiver[50];
    char text[MAX_TEXT_LEN];
} Message;

extern Message messages[MAX_MESSAGES];
extern int message_count;

void send_message(const char *sender, const char *receiver, const char *text);
void print_conversation(const char *user1, const char *user2);

#endif

