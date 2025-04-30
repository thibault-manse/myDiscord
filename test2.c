#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <libpq-fe.h> //inclure fichier .vscode pour qu'elle marche

#define PEPPER "S3cR3tP3pp3r!"

int choice = 0;

void generate_salt(char *salt, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;
    for (size_t i = 0; i < length - 1; i++) {
        salt[i] = charset[rand() % charset_size];
    }
    salt[length - 1] = '\0';
}

void hash_password(const char *password, const char *salt, char *output_hash) {
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s%s", password, salt, PEPPER);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output_hash + (i * 2), "%02x", hash[i]);
    }
    output_hash[64] = '\0'; // SHA256 donne 64 caractères en hexadécimal
}

int check_mail(char *mail){
    char *arobase = strchr(mail, '@');
    if (arobase == NULL)
        return 0;
    
    char *point = strrchr(mail, '.');
    if (point == NULL)
        return 0;

    if(strcmp(point, ".com") == 0 || strcmp(point, ".fr") == 0 || strcmp(point, ".org") == 0 || strcmp(point, ".net") == 0)
        return 1;

    return 0;
}

int connection(){
    srand(time(NULL)); // Initialiser random pour le salt

    const char *conninfo = "dbname=mydiscord user=postgres password=dvjmfr5e host=localhost port=5432";

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    printf("Connexion réussie à PostgreSQL !\n");

    printf("Bienvenur sur myDiscord ! Voulez vous vous connecté ? (1) ou crée un compte ? (2) : ");
    scanf("%d", &choice);

    if (choice == 1){
        char pseudo[50];
        char password[50];
        char hashed_password[65];
        char salt[255];

        printf("Rentré votre pseudo : ");
        scanf("%s", pseudo);

        printf("Rentré votre mot de passe : ");
        scanf("%s", password);

        const char *paramValues1[1] = {pseudo};

        PGresult *res1 = PQexecParams(conn,
            "SELECT salt FROM utilisateurs WHERE pseudo = $1",
            1, NULL, paramValues1, NULL, NULL, 0);

        if (PQresultStatus(res1) != PGRES_TUPLES_OK || PQntuples(res1) == 0) {
            fprintf(stderr, "Pseudo inconnu ou erreur : %s", PQerrorMessage(conn));
            PQclear(res1);
            PQfinish(conn);
            return 1;
        }

        strcpy(salt, PQgetvalue(res1, 0, 0));
        PQclear(res1);

        // Maintenant on hash le mot de passe entré
        hash_password(password, salt, hashed_password);

        const char *paramValues2[2] = {pseudo, hashed_password};

        PGresult *res2 = PQexecParams(conn,
            "SELECT * FROM utilisateurs WHERE pseudo = $1 AND motdepasse = $2",
            2,       // nombre de paramètres
            NULL,    // types des paramètres (NULL = auto)
            paramValues2,
            NULL,    // tailles (NULL = texte)
            NULL,    // formats (NULL = texte)
            0);      // résultat en texte (0), ou en binaire (1)

        if (PQresultStatus(res2) != PGRES_TUPLES_OK) {
            fprintf(stderr, "Erreur dans la requête : %s", PQerrorMessage(conn));
            PQclear(res2);
            PQfinish(conn);
            return 1;
        }

        int nrows = PQntuples(res2);
        int ncols = PQnfields(res2);

        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                printf("%s\t", PQgetvalue(res2, i, j));
            }
            printf("\n");
        }

        PQclear(res2);
        PQfinish(conn);

        return 0;
    }

    else if (choice == 2){
        char new_pseudo[50];
        char new_email[255];
        char new_password[50];
        char salt[16];
        char hashed_password[65];
        int i = 0;

        printf("Rentré votre pseudo : ");
        scanf("%s", new_pseudo);

        while (i == 0){
            printf("Rentré votre email : ");
            scanf("%s", new_email);
    
            if (!check_mail(new_email))
                printf("Adresse mail invalide \n");
            else if (check_mail(new_email))
                i++;
        }

        printf("Rentré votre mot de passe : ");
        scanf("%s", new_password);

        generate_salt(salt, sizeof(salt));
        hash_password(new_password, salt, hashed_password);

        const char *paramValues[4] = {new_pseudo, new_email, hashed_password, salt};

        PGresult *res = PQexecParams(conn,
            "INSERT INTO utilisateurs (pseudo, mail, motdepasse, salt, status_utilisateur, image_url) VALUES ($1, $2, $3, $4, 2, \'mon image\')",
            4,       // nombre de paramètres
            NULL,    // types des paramètres (NULL = auto)
            paramValues,
            NULL,    // tailles (NULL = texte)
            NULL,    // formats (NULL = texte)
            0);      // résultat en texte (0), ou en binaire (1)

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Erreur lors de la création du compte : %s", PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            return 1;
        }

        printf("Bienvenu à toi \n");

        PQclear(res);
        PQfinish(conn);

        return 0;
    }
}

int main() {
    connection();
    return 0;
}


//Début de vérification de mot de passe durant la création d'un compte
//while (i == 0){
//    printf("Confirmer votre mot de passe : ");
//    scanf("%s", check_password);
//    if (&check_password == &new_password)
//        i++;
//    else
//        printf("Ce champ doit etre similaire à votre mot de passe \n");
//}

//Test de base de donnée
//PGresult *test = PQexec(conn, "SELECT * FROM utilisateurs");
//
//if (PQresultStatus(test) != PGRES_TUPLES_OK)
//{
//    fprintf(stderr, "Commande incorrecte: %s", PQerrorMessage(conn));
//    PQclear(test);
//    return 0;
//}
//
//int nrows = PQntuples(test);
//int ncols = PQnfields(test);
//
//for (int i = 0; i < nrows; i++) {
//    for (int j = 0; j < ncols; j++) {
//        printf("%s\t", PQgetvalue(test, i, j));
//    }
//    printf("\n");
//}

//Mot de passe haché Takemi : 68c2e34289238dd10a2e7ccc2a03259b4e19870992a63e3532eff83314a2209a
//Ruirui : a7855c08f28d41eb95262d397fd072fc2fe13c175e2b74efbea0913c94b2140e
//Mochi : 706a05b8ab8cf42149bcdbb6553fa61532af2faf61f9f052c102c05a88f7e8b6
//lola : ea34651dad58283684f32f0c43edb40efe545c52261a5cd930a0b52878b1d663