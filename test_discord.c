#include <gtk/gtk.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>     
#include <stdlib.h>     
#include "messagerie.h"
#include "test_discord.h"  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


GtkWidget *message_box;
GtkWidget *message_entry;
char current_user[50] = "Moi";
char current_friend[50] = "";

GtkWidget *revealer;
gboolean sidebar_visible = TRUE;
GtkCssProvider *css_provider;
GtkWidget *sidebar;
GtkWidget *server_box;
GtkWidget *friends_box;
gboolean showing_friends = FALSE;

int sock;  // socket global pour envoyer/recevoir

gboolean update_ui_with_received_message(gpointer data);


void *receive_messages_thread(void *arg) {
    char buffer[1024];
    int len;

    while ((len = recv(sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[len] = '\0';
        g_idle_add((GSourceFunc)update_ui_with_received_message, g_strdup(buffer));
    }

    close(sock);
    return NULL;
}

gboolean update_ui_with_received_message(gpointer data) {
    const char *text = (const char *)data;

    GtkWidget *msg_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(msg_container, 5);
    gtk_widget_set_margin_bottom(msg_container, 5);
    gtk_widget_set_margin_start(msg_container, 10);
    gtk_widget_set_margin_end(msg_container, 10);

    GtkWidget *label = gtk_label_new(text);
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "message-bubble");
    gtk_label_set_xalign(GTK_LABEL(label), 0);

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 5);

    gtk_box_pack_start(GTK_BOX(msg_container), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(msg_container), separator, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(message_box), msg_container, FALSE, FALSE, 0);

    gtk_widget_show_all(message_box);
    g_free(data);

    return FALSE;
}


// Fonction pour envoyer un message par le réseau
void send_network_message(const char *text) {
    if (sock > 0 && text && strlen(text) > 0) {
        send(sock, text, strlen(text), 0);
    }
}


void toggle_sidebar(GtkButton *button, gpointer user_data) {
    sidebar_visible = !sidebar_visible;
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), sidebar_visible);
}

GtkWidget* create_circle_label(const gchar *text) {
    GtkWidget *event_box = gtk_event_box_new();
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_size_request(event_box, 50, 50);
    gtk_style_context_add_class(gtk_widget_get_style_context(event_box), "circle");
    gtk_container_add(GTK_CONTAINER(event_box), label);

    return event_box;
}

GtkWidget* create_circle_label_with_callback(const gchar *text, GCallback callback) {
    GtkWidget *event_box = create_circle_label(text);

    if (callback) {
        g_signal_connect(event_box, "button-press-event", callback, (gpointer)text);
    }

    return event_box;
}

gboolean on_friend_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    const gchar *friend_name = user_data;
    strncpy(current_friend, friend_name, sizeof(current_friend));
    update_conversation();
    return TRUE;
}

void toggle_friends(GtkButton *button, gpointer user_data) {
    showing_friends = !showing_friends;

    gtk_widget_set_visible(server_box, !showing_friends);
    gtk_widget_set_visible(friends_box, showing_friends);

    if (showing_friends) {
        gtk_button_set_label(GTK_BUTTON(button), "Serveurs");
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "Amis");
    }
}

GtkWidget* create_friends_list() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Alice"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Bob"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Charlie"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Diana"), FALSE, FALSE, 5);

    return box;
}

void update_conversation() {
    GList *children = gtk_container_get_children(GTK_CONTAINER(message_box));
    for (GList *iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    for (int i = message_count - 1; i >= 0; i--) {

        if ((strcmp(messages[i].sender, current_user) == 0 && strcmp(messages[i].receiver, current_friend) == 0) ||
            (strcmp(messages[i].sender, current_friend) == 0 && strcmp(messages[i].receiver, current_user) == 0)) {

            char buffer[1100];
            snprintf(buffer, sizeof(buffer), "%s : %s", messages[i].sender, messages[i].text);

            GtkWidget *msg_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_set_margin_top(msg_container, 5);
            gtk_widget_set_margin_bottom(msg_container, 5);
            gtk_widget_set_margin_start(msg_container, 10);
            gtk_widget_set_margin_end(msg_container, 10);

            GtkWidget *label = gtk_label_new(buffer);
            gtk_style_context_add_class(gtk_widget_get_style_context(label), "message-bubble");
            gtk_label_set_xalign(GTK_LABEL(label), 0);  

            GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_widget_set_margin_top(separator, 5);

            gtk_box_pack_start(GTK_BOX(msg_container), label, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(msg_container), separator, FALSE, FALSE, 0);

            gtk_box_pack_end(GTK_BOX(message_box), msg_container, FALSE, FALSE, 0); 
        }
    }

    gtk_widget_show_all(message_box);

    // Scroll automatique vers le bas
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(message_box)));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
}

void send_message_clicked(GtkButton *button, gpointer user_data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(message_entry));
    if (strlen(text) == 0) return;

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s:%s:%s", current_user, current_friend, text);

    send_network_message(buffer);  
    gtk_entry_set_text(GTK_ENTRY(message_entry), "");
}


static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window, *main_box, *main_content;
    GtkWidget *top_bar, *label, *content_area, *search_bar, *search_entry;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Discord Style UI");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.19, 0.21, 1});

    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
     "* { transition: all 200ms ease-in-out; }"
     ".circle {"
     "    border-radius: 25px;"
     "    background-color: #40444B;"
     "    color: white;"
     "    font-weight: bold;"
     "    font-size: 16px;"
     "    text-align: center;"
     "}"
     ".message-bubble {"
     "    background-color:rgb(167, 160, 160);" /* Gris clair */
     "    color: black;" 
     "    padding: 10px;"
     "    border-radius: 10px;"
     "    margin-bottom: 5px;"
     "    box-shadow: 0 0 2px rgba(0, 0, 0, 0.3);"
     "}"
     , -1, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(sidebar, 300, -1);
    gtk_widget_override_background_color(sidebar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.125, 0.133, 0.145, 1});

    GtkWidget *amis_button = gtk_button_new_with_label("Amis");
    gtk_widget_override_background_color(amis_button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.2, 0.4, 0.8, 1});
    gtk_widget_override_color(amis_button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0, 0, 0, 1});
    gtk_box_pack_start(GTK_BOX(sidebar), amis_button, FALSE, FALSE, 20);
    g_signal_connect(amis_button, "clicked", G_CALLBACK(toggle_friends), amis_button);

    server_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    friends_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Alice", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Bob", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Charlie", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Diana", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(spacer, -1, 40);
    gtk_box_pack_start(GTK_BOX(sidebar), spacer, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("A"), FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("B"), FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("C"), FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(sidebar), server_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sidebar), friends_box, FALSE, FALSE, 0);
    gtk_widget_set_visible(friends_box, FALSE);

    revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);
    gtk_container_add(GTK_CONTAINER(revealer), sidebar);
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), revealer, FALSE, FALSE, 0);

    main_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), main_content, TRUE, TRUE, 0);

    top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_size_request(top_bar, -1, 50);
    gtk_widget_override_background_color(top_bar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.14, 0.15, 0.17, 1});
    gtk_box_pack_start(GTK_BOX(main_content), top_bar, FALSE, FALSE, 0);

    GtkWidget *toggle_btn = gtk_button_new_with_label("☰");
    gtk_widget_set_size_request(toggle_btn, 40, 40);
    g_signal_connect(toggle_btn, "clicked", G_CALLBACK(toggle_sidebar), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), toggle_btn, FALSE, FALSE, 10);

    label = gtk_label_new("Accueil");
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1, 1, 1, 1});
    gtk_box_pack_start(GTK_BOX(top_bar), label, FALSE, FALSE, 0);

    search_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_size_request(search_bar, -1, 40);
    gtk_widget_override_background_color(search_bar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.22, 0.23, 0.25, 1});
    gtk_box_pack_start(GTK_BOX(main_content), search_bar, FALSE, FALSE, 10);

    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Rechercher...");
    gtk_widget_set_size_request(search_entry, 300, 30);
    gtk_box_pack_start(GTK_BOX(search_bar), search_entry, FALSE, FALSE, 10);

    content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_override_background_color(content_area, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.19, 0.21, 1});
    gtk_box_pack_start(GTK_BOX(main_content), content_area, TRUE, TRUE, 0);

    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_pack_start(GTK_BOX(content_area), scroller, TRUE, TRUE, 5);

    message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(message_box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(message_box, GTK_ALIGN_END); 
    gtk_container_add(GTK_CONTAINER(scroller), message_box);

    GtkWidget *entry_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(entry_row), message_entry, TRUE, TRUE, 5);

    GtkWidget *send_btn = gtk_button_new_with_label("Envoyer");
    g_signal_connect(send_btn, "clicked", G_CALLBACK(send_message_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(entry_row), send_btn, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(content_area), entry_row, FALSE, FALSE, 5);

    gtk_widget_show_all(window);
}

void start_gui(int sockfd) {
    sock = sockfd;  // sauvegarde du socket

    GtkApplication *app;
    app = gtk_application_new("com.example.discordstyled.ui", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages_thread, NULL);
    pthread_detach(recv_thread);

    g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
}

