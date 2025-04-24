#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

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

    // Exécution d'une requête
    char pseudo[50];
    char motdepasse[50];
    printf("Rentré votre pseudo : ");
    scanf("%s", pseudo);

    printf("Rentré votre mot de passe : ");
    scanf("%s", motdepasse);

    const char *paramValues[2] = {pseudo, motdepasse};

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