--
-- PostgreSQL database dump
--

-- Dumped from database version 16.8 (Ubuntu 16.8-0ubuntu0.24.04.1)
-- Dumped by pg_dump version 16.8 (Ubuntu 16.8-0ubuntu0.24.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: fichier; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.fichier (
    fichier_id integer NOT NULL,
    message_id integer,
    url_fichier text NOT NULL,
    type_fichier character varying(255) NOT NULL
);


ALTER TABLE public.fichier OWNER TO postgres;

--
-- Name: fichier_fichier_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.fichier_fichier_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.fichier_fichier_id_seq OWNER TO postgres;

--
-- Name: fichier_fichier_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.fichier_fichier_id_seq OWNED BY public.fichier.fichier_id;


--
-- Name: membre_banni; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.membre_banni (
    utilisateur_id integer NOT NULL,
    serveur_id integer NOT NULL
);


ALTER TABLE public.membre_banni OWNER TO postgres;

--
-- Name: membre_serveur; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.membre_serveur (
    utilisateur_id integer NOT NULL,
    serveur_id integer NOT NULL
);


ALTER TABLE public.membre_serveur OWNER TO postgres;

--
-- Name: messages; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.messages (
    message_id integer NOT NULL,
    tchat_id integer,
    envoyeur_id integer,
    contenu text NOT NULL,
    date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    reponse integer,
    reaction character varying(255)
);


ALTER TABLE public.messages OWNER TO postgres;

--
-- Name: messages_message_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.messages_message_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.messages_message_id_seq OWNER TO postgres;

--
-- Name: messages_message_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.messages_message_id_seq OWNED BY public.messages.message_id;


--
-- Name: roles; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.roles (
    role_id integer NOT NULL,
    serveur_id integer,
    nom character varying(50) NOT NULL,
    couleur character varying(255) NOT NULL,
    permissions bigint NOT NULL,
    position_role integer NOT NULL
);


ALTER TABLE public.roles OWNER TO postgres;

--
-- Name: roles_membre; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.roles_membre (
    role_id integer NOT NULL,
    membre_id integer NOT NULL,
    serveur_id integer NOT NULL
);


ALTER TABLE public.roles_membre OWNER TO postgres;

--
-- Name: roles_role_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.roles_role_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.roles_role_id_seq OWNER TO postgres;

--
-- Name: roles_role_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.roles_role_id_seq OWNED BY public.roles.role_id;


--
-- Name: serveur; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.serveur (
    serveur_id integer NOT NULL,
    nom character varying(50) NOT NULL,
    createur_id integer,
    "privé" integer NOT NULL,
    image_url text NOT NULL
);


ALTER TABLE public.serveur OWNER TO postgres;

--
-- Name: serveur_serveur_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.serveur_serveur_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.serveur_serveur_id_seq OWNER TO postgres;

--
-- Name: serveur_serveur_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.serveur_serveur_id_seq OWNED BY public.serveur.serveur_id;


--
-- Name: tchat; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.tchat (
    tchat_id integer NOT NULL,
    nom character varying(50) NOT NULL,
    serveur_id integer
);


ALTER TABLE public.tchat OWNER TO postgres;

--
-- Name: tchat_tchat_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.tchat_tchat_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.tchat_tchat_id_seq OWNER TO postgres;

--
-- Name: tchat_tchat_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.tchat_tchat_id_seq OWNED BY public.tchat.tchat_id;


--
-- Name: utilisateurs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.utilisateurs (
    utilisateur_id integer NOT NULL,
    pseudo character varying(50) NOT NULL,
    mail character varying(255) NOT NULL,
    motdepasse character varying(255) NOT NULL,
    salt character varying(255) NOT NULL,
    biographie text,
    status_utilisateur integer NOT NULL,
    image_url text NOT NULL
);


ALTER TABLE public.utilisateurs OWNER TO postgres;

--
-- Name: utilisateurs_utilisateur_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.utilisateurs_utilisateur_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.utilisateurs_utilisateur_id_seq OWNER TO postgres;

--
-- Name: utilisateurs_utilisateur_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.utilisateurs_utilisateur_id_seq OWNED BY public.utilisateurs.utilisateur_id;


--
-- Name: fichier fichier_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.fichier ALTER COLUMN fichier_id SET DEFAULT nextval('public.fichier_fichier_id_seq'::regclass);


--
-- Name: messages message_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.messages ALTER COLUMN message_id SET DEFAULT nextval('public.messages_message_id_seq'::regclass);


--
-- Name: roles role_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles ALTER COLUMN role_id SET DEFAULT nextval('public.roles_role_id_seq'::regclass);


--
-- Name: serveur serveur_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.serveur ALTER COLUMN serveur_id SET DEFAULT nextval('public.serveur_serveur_id_seq'::regclass);


--
-- Name: tchat tchat_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tchat ALTER COLUMN tchat_id SET DEFAULT nextval('public.tchat_tchat_id_seq'::regclass);


--
-- Name: utilisateurs utilisateur_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs ALTER COLUMN utilisateur_id SET DEFAULT nextval('public.utilisateurs_utilisateur_id_seq'::regclass);


--
-- Data for Name: fichier; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.fichier (fichier_id, message_id, url_fichier, type_fichier) FROM stdin;
\.


--
-- Data for Name: membre_banni; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.membre_banni (utilisateur_id, serveur_id) FROM stdin;
\.


--
-- Data for Name: membre_serveur; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.membre_serveur (utilisateur_id, serveur_id) FROM stdin;
1	1
1	2
2	1
2	2
3	1
\.


--
-- Data for Name: messages; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.messages (message_id, tchat_id, envoyeur_id, contenu, date, reponse, reaction) FROM stdin;
\.


--
-- Data for Name: roles; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.roles (role_id, serveur_id, nom, couleur, permissions, position_role) FROM stdin;
\.


--
-- Data for Name: roles_membre; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.roles_membre (role_id, membre_id, serveur_id) FROM stdin;
\.


--
-- Data for Name: serveur; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.serveur (serveur_id, nom, createur_id, "privé", image_url) FROM stdin;
1	Premier serveur	1	0	Une image
2	tchat privé	1	1	une image
\.


--
-- Data for Name: tchat; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.tchat (tchat_id, nom, serveur_id) FROM stdin;
1	general	1
2	connerie	1
3	privé	2
\.


--
-- Data for Name: utilisateurs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.utilisateurs (utilisateur_id, pseudo, mail, motdepasse, salt, biographie, status_utilisateur, image_url) FROM stdin;
1	Takemi13	Biduledu13@gmail.com	68c2e34289238dd10a2e7ccc2a03259b4e19870992a63e3532eff83314a2209a	xA7pL2kZ	Je sais pas quoi mettre	2	url_image
2	Ruirui@Pilaf	ZoumTallon@gmail.com	a7855c08f28d41eb95262d397fd072fc2fe13c175e2b74efbea0913c94b2140e	Vj8Yc3Wn	Je vend du viagra à 6 centimes	2	url_image
3	Mochi	Zebullon82@gmail.com	706a05b8ab8cf42149bcdbb6553fa61532af2faf61f9f052c102c05a88f7e8b6	Lz9Qe8tU	NI KONI KONIIIIIIIIIIIIII :3	2	url_image
4	lola03	lola.enigma@gmail.com	ea34651dad58283684f32f0c43edb40efe545c52261a5cd930a0b52878b1d663	ABcD1234	\N	2	url_image
6	sora	fandekh@gmail.com	3d8060a128d4ba72a63ceefe15958ba90e44ce145ab8cfb87d107e5a7a1a06d2	qe3LPHwgLEJmDTy	\N	2	mon image
\.


--
-- Name: fichier_fichier_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.fichier_fichier_id_seq', 1, false);


--
-- Name: messages_message_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.messages_message_id_seq', 1, false);


--
-- Name: roles_role_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.roles_role_id_seq', 1, false);


--
-- Name: serveur_serveur_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.serveur_serveur_id_seq', 1, false);


--
-- Name: tchat_tchat_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.tchat_tchat_id_seq', 1, false);


--
-- Name: utilisateurs_utilisateur_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.utilisateurs_utilisateur_id_seq', 6, true);


--
-- Name: fichier fichier_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.fichier
    ADD CONSTRAINT fichier_pkey PRIMARY KEY (fichier_id);


--
-- Name: membre_banni membre_banni_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.membre_banni
    ADD CONSTRAINT membre_banni_pkey PRIMARY KEY (utilisateur_id, serveur_id);


--
-- Name: membre_serveur membre_serveur_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.membre_serveur
    ADD CONSTRAINT membre_serveur_pkey PRIMARY KEY (utilisateur_id, serveur_id);


--
-- Name: messages messages_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_pkey PRIMARY KEY (message_id);


--
-- Name: roles_membre roles_membre_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles_membre
    ADD CONSTRAINT roles_membre_pkey PRIMARY KEY (role_id, membre_id, serveur_id);


--
-- Name: roles roles_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_pkey PRIMARY KEY (role_id);


--
-- Name: serveur serveur_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.serveur
    ADD CONSTRAINT serveur_pkey PRIMARY KEY (serveur_id);


--
-- Name: tchat tchat_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tchat
    ADD CONSTRAINT tchat_pkey PRIMARY KEY (tchat_id);


--
-- Name: utilisateurs utilisateurs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs
    ADD CONSTRAINT utilisateurs_pkey PRIMARY KEY (utilisateur_id);


--
-- Name: fichier fichier_message_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.fichier
    ADD CONSTRAINT fichier_message_id_fkey FOREIGN KEY (message_id) REFERENCES public.messages(message_id);


--
-- Name: membre_banni membre_banni_serveur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.membre_banni
    ADD CONSTRAINT membre_banni_serveur_id_fkey FOREIGN KEY (serveur_id) REFERENCES public.serveur(serveur_id);


--
-- Name: membre_banni membre_banni_utilisateur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.membre_banni
    ADD CONSTRAINT membre_banni_utilisateur_id_fkey FOREIGN KEY (utilisateur_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: membre_serveur membre_serveur_serveur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.membre_serveur
    ADD CONSTRAINT membre_serveur_serveur_id_fkey FOREIGN KEY (serveur_id) REFERENCES public.serveur(serveur_id);


--
-- Name: messages messages_envoyeur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_envoyeur_id_fkey FOREIGN KEY (envoyeur_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: messages messages_reponse_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_reponse_fkey FOREIGN KEY (reponse) REFERENCES public.messages(message_id);


--
-- Name: messages messages_tchat_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.messages
    ADD CONSTRAINT messages_tchat_id_fkey FOREIGN KEY (tchat_id) REFERENCES public.serveur(serveur_id);


--
-- Name: roles_membre roles_membre_membre_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles_membre
    ADD CONSTRAINT roles_membre_membre_id_fkey FOREIGN KEY (membre_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: roles_membre roles_membre_role_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles_membre
    ADD CONSTRAINT roles_membre_role_id_fkey FOREIGN KEY (role_id) REFERENCES public.roles(role_id);


--
-- Name: roles_membre roles_membre_serveur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles_membre
    ADD CONSTRAINT roles_membre_serveur_id_fkey FOREIGN KEY (serveur_id) REFERENCES public.serveur(serveur_id);


--
-- Name: roles roles_serveur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_serveur_id_fkey FOREIGN KEY (serveur_id) REFERENCES public.serveur(serveur_id);


--
-- Name: tchat tchat_serveur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tchat
    ADD CONSTRAINT tchat_serveur_id_fkey FOREIGN KEY (serveur_id) REFERENCES public.serveur(serveur_id);


--
-- PostgreSQL database dump complete
--

