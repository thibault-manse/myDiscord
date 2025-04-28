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

// Widgets de l'interface utilisateur pour afficher les messages et écrire de nouveaux messages.
GtkWidget *message_box;      // Conteneur pour afficher la liste des messages.
GtkWidget *message_entry;    // Zone de saisie pour écrire un message.

// Informations sur l'utilisateur et l'ami en cours de communication.
char current_user[50] = "Moi";       // Nom de l'utilisateur actuel (par défaut "Moi").
char current_friend[50] = "";       // Nom de l'ami sélectionné pour la conversation.

// Gestion des éléments de l'interface utilisateur.
GtkWidget *revealer;                 // Composant pour afficher ou masquer des parties de l'interface.
gboolean sidebar_visible = TRUE;     // État de visibilité de la barre latérale.
GtkCssProvider *css_provider;        // Fournisseur de styles CSS pour personnaliser l'apparence.
GtkWidget *sidebar;                  // Conteneur pour la barre latérale.
GtkWidget *server_box;               // Conteneur pour afficher les serveurs.
GtkWidget *friends_box;              // Conteneur pour afficher la liste des amis.
gboolean showing_friends = FALSE;    // État indiquant si la liste des amis est actuellement affichée.

// Gestion du socket réseau.
int sock;  // Socket global utilisé pour envoyer et recevoir des messages.

// Déclaration de la fonction pour mettre à jour l'interface avec un message reçu.
gboolean update_ui_with_received_message(gpointer data);

// Thread pour recevoir les messages provenant du réseau.
void *receive_messages_thread(void *arg) {
    char buffer[1024];    // Tampon pour stocker les données reçues.
    int len;              // Longueur des données reçues.

    // Boucle pour recevoir les messages.
    while ((len = recv(sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[len] = '\0';   // Ajout du terminateur NULL pour la chaîne.
        
        // Ajout d'une tâche à l'interface utilisateur pour mettre à jour les messages reçus.
        g_idle_add((GSourceFunc)update_ui_with_received_message, g_strdup(buffer));
    }

    // Fermeture du socket une fois terminé.
    close(sock);
    return NULL;
}

// Fonction pour mettre à jour l'interface avec le message reçu.
gboolean update_ui_with_received_message(gpointer data) {
    const char *text = (const char *)data;   // Conversion des données en texte.

    // Création d'un conteneur pour le message reçu.
    GtkWidget *msg_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(msg_container, 5);
    gtk_widget_set_margin_bottom(msg_container, 5);
    gtk_widget_set_margin_start(msg_container, 10);
    gtk_widget_set_margin_end(msg_container, 10);

    // Création d'un label pour afficher le texte du message.
    GtkWidget *label = gtk_label_new(text);
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "message-bubble"); // Style personnalisé.
    gtk_label_set_xalign(GTK_LABEL(label), 0); // Alignement horizontal à gauche.

    // Ajout d'un séparateur pour séparer visuellement les messages.
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 5);

    // Ajout du label et du séparateur au conteneur.
    gtk_box_pack_start(GTK_BOX(msg_container), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(msg_container), separator, FALSE, FALSE, 0);

    // Ajout du conteneur à la boîte principale pour les messages.
    gtk_box_pack_end(GTK_BOX(message_box), msg_container, FALSE, FALSE, 0);

    // Affichage de tous les widgets ajoutés.
    gtk_widget_show_all(message_box);

    // Libération des données utilisées.
    g_free(data);

    return FALSE; // Retourne FALSE pour indiquer que la tâche a été exécutée.
}

// Fonction pour envoyer un message par le réseau
void send_network_message(const char *text) {
    // Vérifie que le socket est valide, que le texte existe et qu'il n'est pas vide.
    if (sock > 0 && text && strlen(text) > 0) {
        // Envoie le message via le socket réseau.
        send(sock, text, strlen(text), 0);
    }
}

// Fonction pour basculer la visibilité de la barre latérale.
void toggle_sidebar(GtkButton *button, gpointer user_data) {
    sidebar_visible = !sidebar_visible; // Inverse l'état de visibilité de la barre latérale.
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), sidebar_visible); // Met à jour l'interface pour afficher/masquer.
}

// Fonction pour créer un label circulaire.
GtkWidget* create_circle_label(const gchar *text) {
    GtkWidget *event_box = gtk_event_box_new(); // Crée une boîte d'événement pour le label.
    GtkWidget *label = gtk_label_new(text);     // Crée le label avec le texte fourni.
    gtk_widget_set_size_request(event_box, 50, 50); // Définit la taille du conteneur circulaire.
    gtk_style_context_add_class(gtk_widget_get_style_context(event_box), "circle"); // Applique le style CSS "circle".
    gtk_container_add(GTK_CONTAINER(event_box), label); // Ajoute le label dans le conteneur circulaire.

    return event_box; // Retourne le widget créé.
}

// Fonction pour créer un label circulaire avec un callback (gestion des clics).
GtkWidget* create_circle_label_with_callback(const gchar *text, GCallback callback) {
    GtkWidget *event_box = create_circle_label(text); // Crée le label circulaire.

    // Si un callback est défini, connecte l'événement de clic du label à ce callback.
    if (callback) {
        g_signal_connect(event_box, "button-press-event", callback, (gpointer)text);
    }

    return event_box; // Retourne le widget circulaire avec le gestionnaire d'événements.
}

// Fonction appelée lorsqu'un ami est cliqué.
gboolean on_friend_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    const gchar *friend_name = user_data; // Récupère le nom de l'ami depuis les données utilisateur.
    strncpy(current_friend, friend_name, sizeof(current_friend)); // Met à jour l'ami en cours.
    update_conversation(); // Met à jour l'interface pour afficher la conversation avec cet ami.
    return TRUE; // Retourne TRUE pour indiquer que l'événement a été géré.
}

// Fonction pour basculer entre la vue des amis et des serveurs.
void toggle_friends(GtkButton *button, gpointer user_data) {
    showing_friends = !showing_friends; // Inverse l'état d'affichage des amis/serveurs.

    // Affiche ou masque les conteneurs appropriés selon l'état.
    gtk_widget_set_visible(server_box, !showing_friends);
    gtk_widget_set_visible(friends_box, showing_friends);

    // Met à jour le texte du bouton en fonction de l'état.
    if (showing_friends) {
        gtk_button_set_label(GTK_BUTTON(button), "Serveurs");
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "Amis");
    }
}

// Fonction pour créer une liste d'amis.
GtkWidget* create_friends_list() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Crée un conteneur vertical pour les amis.

    // Ajoute les amis à la liste avec des labels circulaires.
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Alice"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Bob"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Charlie"), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), create_circle_label("Diana"), FALSE, FALSE, 5);

    return box; // Retourne le conteneur contenant la liste d'amis.
}

// Fonction pour mettre à jour la conversation en fonction de l'ami sélectionné.
void update_conversation() {
    GList *children = gtk_container_get_children(GTK_CONTAINER(message_box)); // Récupère tous les widgets enfants dans message_box.
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data)); // Détruit chaque widget enfant.
    }
    g_list_free(children); // Libère la liste des widgets.

    // Parcours des messages pour afficher ceux appartenant à la conversation actuelle.
    for (int i = message_count - 1; i >= 0; i--) {
        if ((strcmp(messages[i].sender, current_user) == 0 && strcmp(messages[i].receiver, current_friend) == 0) ||
            (strcmp(messages[i].sender, current_friend) == 0 && strcmp(messages[i].receiver, current_user) == 0)) {
            
            char buffer[1100];
            snprintf(buffer, sizeof(buffer), "%s : %s", messages[i].sender, messages[i].text); // Formate le texte du message.

            GtkWidget *msg_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // Crée un conteneur pour le message.
            gtk_widget_set_margin_top(msg_container, 5);
            gtk_widget_set_margin_bottom(msg_container, 5);
            gtk_widget_set_margin_start(msg_container, 10);
            gtk_widget_set_margin_end(msg_container, 10);

            GtkWidget *label = gtk_label_new(buffer); // Crée un label pour le texte du message.
            gtk_style_context_add_class(gtk_widget_get_style_context(label), "message-bubble");
            gtk_label_set_xalign(GTK_LABEL(label), 0);

            GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL); // Crée un séparateur pour séparer visuellement les messages.
            gtk_widget_set_margin_top(separator, 5);

            gtk_box_pack_start(GTK_BOX(msg_container), label, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(msg_container), separator, FALSE, FALSE, 0);

            gtk_box_pack_end(GTK_BOX(message_box), msg_container, FALSE, FALSE, 0);
        }
    }

    gtk_widget_show_all(message_box); // Affiche tous les widgets ajoutés à message_box.

    // Scroll automatique vers le bas pour afficher les derniers messages.
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(message_box)));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
}

// Fonction déclenchée lorsque l'utilisateur clique sur le bouton "Envoyer".
void send_message_clicked(GtkButton *button, gpointer user_data) {
    // Récupère le texte écrit dans la zone d'entrée (message_entry).
    const char *text = gtk_entry_get_text(GTK_ENTRY(message_entry));
    
    // Vérifie si le texte est vide. Si oui, la fonction quitte sans rien faire.
    if (strlen(text) == 0) return;

    // Prépare un buffer avec le format : "expéditeur:destinataire:message".
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s:%s:%s", current_user, current_friend, text);

    // Envoie le message via le réseau.
    send_network_message(buffer);

    // Efface le texte de la zone d'entrée après l'envoi.
    gtk_entry_set_text(GTK_ENTRY(message_entry), "");
}

// Fonction principale, appelée au démarrage de l'application GTK.
static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window, *main_box, *main_content;
    GtkWidget *top_bar, *label, *content_area, *search_bar, *search_entry;

    // Création de la fenêtre principale.
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Discord Style UI"); // Définit le titre de la fenêtre.
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);  // Définit la taille par défaut de la fenêtre.
    
    // Modifie la couleur d'arrière-plan de la fenêtre.
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.19, 0.21, 1});

    // Initialisation du fournisseur CSS pour les styles personnalisés.
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

    // Applique les styles CSS à l'écran par défaut.
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Création de la boîte principale (conteneur horizontal).
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Création de la barre latérale.
    sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(sidebar, 300, -1);
    gtk_widget_override_background_color(sidebar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.125, 0.133, 0.145, 1});

    // Bouton pour basculer entre amis/serveurs.
    GtkWidget *amis_button = gtk_button_new_with_label("Amis");
    gtk_widget_override_background_color(amis_button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.2, 0.4, 0.8, 1});
    gtk_widget_override_color(amis_button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0, 0, 0, 1});
    gtk_box_pack_start(GTK_BOX(sidebar), amis_button, FALSE, FALSE, 20);
    g_signal_connect(amis_button, "clicked", G_CALLBACK(toggle_friends), amis_button);

    // Création des boîtes pour les serveurs et les amis.
    server_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    friends_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Ajout des amis avec des callbacks pour cliquer.
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Alice", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Bob", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Charlie", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(friends_box), create_circle_label_with_callback("Diana", G_CALLBACK(on_friend_clicked)), FALSE, FALSE, 5);

    // Ajout des serveurs.
    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("A"), FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("B"), FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(server_box), create_circle_label("C"), FALSE, FALSE, 10);

    // Ajout des boîtes (serveurs/amis) dans la barre latérale.
    gtk_box_pack_start(GTK_BOX(sidebar), server_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sidebar), friends_box, FALSE, FALSE, 0);
    gtk_widget_set_visible(friends_box, FALSE); // Cache la boîte des amis par défaut.

    // Utilisation d'un revealer pour des transitions élégantes dans la barre latérale.
    revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);
    gtk_container_add(GTK_CONTAINER(revealer), sidebar);
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), revealer, FALSE, FALSE, 0);

    // Création de la zone principale pour le contenu.
    main_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), main_content, TRUE, TRUE, 0);

    // Création de la barre supérieure (header).
    top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_size_request(top_bar, -1, 50);
    gtk_widget_override_background_color(top_bar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.14, 0.15, 0.17, 1});
    gtk_box_pack_start(GTK_BOX(main_content), top_bar, FALSE, FALSE, 0);

    // Bouton pour afficher/masquer la barre latérale.
    GtkWidget *toggle_btn = gtk_button_new_with_label("☰");
    gtk_widget_set_size_request(toggle_btn, 40, 40);
    g_signal_connect(toggle_btn, "clicked", G_CALLBACK(toggle_sidebar), NULL);
    gtk_box_pack_start(GTK_BOX(top_bar), toggle_btn, FALSE, FALSE, 10);

    // Label "Accueil" dans la barre supérieure.
    label = gtk_label_new("Accueil");
    gtk_widget_override_color(label, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1, 1, 1, 1});
    gtk_box_pack_start(GTK_BOX(top_bar), label, FALSE, FALSE, 0);

    // Barre de recherche.
    search_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_size_request(search_bar, -1, 40);
    gtk_widget_override_background_color(search_bar, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.22, 0.23, 0.25, 1});
    gtk_box_pack_start(GTK_BOX(main_content), search_bar, FALSE, FALSE, 10);

    // Champ de saisie pour la recherche.
    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Rechercher...");
    gtk_widget_set_size_request(search_entry, 300, 30);
    gtk_box_pack_start(GTK_BOX(search_bar), search_entry, FALSE, FALSE, 10);

    // Zone principale pour afficher les conversations/messages.
    content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_override_background_color(content_area, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.18, 0.19, 0.21, 1});
    gtk_box_pack_start(GTK_BOX(main_content), content_area, TRUE, TRUE, 0);

    // Scroller pour les messages.
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_pack_start(GTK_BOX(content_area), scroller, TRUE, TRUE, 5);

    // Boîte pour afficher les messages.
    message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_halign(message_box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(message_box, GTK_ALIGN_END);
    gtk_container_add(GTK_CONTAINER(scroller), message_box);

    // Ligne pour la saisie et l'envoi de messages.
     GtkWidget *entry_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Zone d'entrée pour écrire les messages.
    message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(entry_row), message_entry, TRUE, TRUE, 5);
    
    // Bouton "Envoyer" pour transmettre le message via la fonction associée.
    GtkWidget *send_btn = gtk_button_new_with_label("Envoyer");
    g_signal_connect(send_btn, "clicked", G_CALLBACK(send_message_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(entry_row), send_btn, FALSE, FALSE, 5);
    
    // Ajout de la ligne de saisie des messages à la zone de contenu.
    gtk_box_pack_start(GTK_BOX(content_area), entry_row, FALSE, FALSE, 5);
    
    // Affiche tous les composants de la fenêtre.
    gtk_widget_show_all(window);
    }

// Fonction principale pour démarrer l'interface graphique de l'application.
void start_gui(int sockfd) {
    // Sauvegarde du socket dans une variable globale pour l'utiliser dans d'autres fonctions.
    sock = sockfd;

    // Création d'une nouvelle application GTK.
    GtkApplication *app;
    app = gtk_application_new("com.example.discordstyled.ui", G_APPLICATION_DEFAULT_FLAGS); // Nom unique pour l'application.

    // Connexion du signal "activate" à la fonction `activate`, qui initialise l'interface utilisateur.
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Création d'un thread séparé pour recevoir les messages du réseau.
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages_thread, NULL); // Le thread exécute `receive_messages_thread`.
    pthread_detach(recv_thread); // Détache le thread pour qu'il fonctionne indépendamment.

    // Démarre l'application GTK et entre dans la boucle principale de l'application.
    g_application_run(G_APPLICATION(app), 0, NULL);

    // Libère la mémoire associée à l'application GTK après la fin de son exécution.
    g_object_unref(app);
}