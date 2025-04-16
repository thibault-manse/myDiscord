-- Database: myDiscord

-- DROP DATABASE IF EXISTS "myDiscord";

--CREATE DATABASE "myDiscord"
--    WITH
--    OWNER = postgres
--    ENCODING = 'UTF8'
--    LC_COLLATE = 'fr-FR'
--    LC_CTYPE = 'fr-FR'
--    LOCALE_PROVIDER = 'libc'
--    TABLESPACE = pg_default
--    CONNECTION LIMIT = -1
--    IS_TEMPLATE = False;

-- Liste des tableaux
DROP TABLE IF EXISTS utilisateurs CASCADE;

CREATE TABLE utilisateurs (
	utilisateur_id SERIAL PRIMARY KEY,
	pseudo VARCHAR(50) NOT NULL,
	mail VARCHAR(255) NOT NULL,
	motdepasse VARCHAR(255) NOT NULL,
	biographie TEXT,
	image_url TEXT NOT NULL
);

INSERT INTO utilisateurs VALUES (1,'Takemi13', 'Biduledu13@gmail.com', '12345', 'Je sais pas quoi mettre', 'Mon image'),
(2, 'Ruirui@Pilaf', 'ZoumTallon@gmail.com', '12345', 'Je vend du viagra à 6 centimes', 'Mon image');

DROP TABLE IF EXISTS serveur CASCADE;

CREATE TABLE serveur(
	serveur_id SERIAL PRIMARY KEY,
	nom VARCHAR(50) NOT NULL,
	utilisateur_id INT REFERENCES utilisateurs (utilisateur_id)
);

INSERT INTO serveur VALUES (1,'Premier serveur', 1);

DROP TABLE IF EXISTS membre_serveur CASCADE;

CREATE TABLE membre_serveur(
	utilisateur_id INT REFERENCES utilisateurs (utilisateur_id),
	serveur_id INT REFERENCES serveur (serveur_id),
	PRIMARY KEY (utilisateur_id, serveur_id)
);

DROP TABLE IF EXISTS tchat CASCADE;

CREATE TABLE tchat (
	tchat_id SERIAL PRIMARY KEY,
	nom VARCHAR(50) NOT NULL,
	serveur_id INT REFERENCES serveur(serveur_id),
	position_tchat INT NOT NULL
);

DROP TABLE IF EXISTS messages CASCADE;

CREATE TABLE messages (
	message_id SERIAL PRIMARY KEY,
	serveur_id INT REFERENCES serveur(serveur_id),
	envoyeur_id INT REFERENCES utilisateurs(utilisateur_id),
	contenu TEXT NOT NULL,
	date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	reponse INT REFERENCES messages(message_id),
	reaction VARCHAR(255)
);

DROP TABLE IF EXISTS roles CASCADE;

CREATE TABLE roles (
	role_id SERIAL PRIMARY KEY,
	serveur_id INT REFERENCES serveur(serveur_id),
	nom VARCHAR(50) NOT NULL,
	couleur VARCHAR(255) NOT NULL,
	permissions BIGINT NOT NULL,
	position_role INT NOT NULL
);

DROP TABLE IF EXISTS roles_membre CASCADE;

CREATE TABLE roles_membre(
	role_id INT REFERENCES roles(role_id),
	membre_id INT REFERENCES utilisateurs(utilisateur_id),
	serveur_id INT REFERENCES serveur(serveur_id),
	PRIMARY KEY (role_id, membre_id, serveur_id)
);

DROP TABLE IF EXISTS fichier CASCADE;

CREATE TABLE fichier(
	fichier_id SERIAL PRIMARY KEY,
	message_id INT REFERENCES messages(message_id),
	url_fichier TEXT NOT NULL,
	type_fichier VARCHAR(255) NOT NULL
);