#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

int choice = 0;

int main() {
    // Chaîne de connexion
    const char *conninfo = "dbname=mydiscord user=postgres password=dvjmfr5e host=localhost port=5432";


    // Connexion à la base
    PGconn *conn = PQconnectdb(conninfo);

    // Vérification de la connexion
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Erreur de connexion : %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    printf("Connexion réussie à PostgreSQL !\n");

    printf("Bienvenur sur myDiscord ! Voulez vous vous connecté ? (1) ou crée un compte ? (2) : ");
    scanf("%d", &choice);

    if (choice == 1){
        // Exécution d'une requête
        char pseudo[50];
        char password[50];
        printf("Rentré votre pseudo : ");
        scanf("%s", pseudo);

        printf("Rentré votre mot de passe : ");
        scanf("%s", password);

        const char *paramValues[2] = {pseudo, password};

        PGresult *res = PQexecParams(conn,
            "SELECT * FROM utilisateurs WHERE pseudo = $1 AND motdepasse = $2",
            2,       // nombre de paramètres
            NULL,    // types des paramètres (NULL = auto)
            paramValues,
            NULL,    // tailles (NULL = texte)
            NULL,    // formats (NULL = texte)
            0);      // résultat en texte (0), ou en binaire (1)

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            fprintf(stderr, "Erreur dans la requête : %s", PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            return 1;
        }

        // Lecture des résultats
        int nrows = PQntuples(res);
        int ncols = PQnfields(res);

        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols; j++) {
                printf("%s\t", PQgetvalue(res, i, j));
            }
            printf("\n");
        }

        // Libération et fermeture
        PQclear(res);
        PQfinish(conn);

        return 0;
    }

    else if (choice == 2){
        char new_pseudo[50];
        char new_email[255];
        char new_password[50];
        //char check_password[50];
        //int i = 0;
        printf("Rentré votre pseudo : ");
        scanf("%s", new_pseudo);

        printf("Rentré votre email : ");
        scanf("%s", new_email);

        printf("Rentré votre mot de passe : ");
        scanf("%s", new_password);

        const char *paramValues[3] = {new_pseudo, new_email, new_password};

        PGresult *res = PQexecParams(conn,
            "INSERT INTO utilisateurs (pseudo, mail, motdepasse, status_utilisateur, image_url) VALUES ($1, $2, $3, 2, \'mon image\')",
            3,       // nombre de paramètres
            NULL,    // types des paramètres (NULL = auto)
            paramValues,
            NULL,    // tailles (NULL = texte)
            NULL,    // formats (NULL = texte)
            0);      // résultat en texte (0), ou en binaire (1)

        printf("Bienvenu à toi \n");

        // Libération et fermeture
        PQclear(res);
        PQfinish(conn);

        return 0;
    }
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