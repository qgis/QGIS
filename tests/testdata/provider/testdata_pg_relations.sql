--
-- Name: relations; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA relations;

--
-- Name: c_amgmt_amgmt_lot; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.c_amgmt_amgmt_lot (
    id integer NOT NULL,
    id_amgmt character varying(80),
    id_amgmt_lot character varying(80)
);


--
-- Name: c_amgmt_amgmt_lot_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.c_amgmt_amgmt_lot_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: c_amgmt_amgmt_lot_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.c_amgmt_amgmt_lot_id_seq OWNED BY relations.c_amgmt_amgmt_lot.id;


--
-- Name: c_batiment_bat_lot; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.c_batiment_bat_lot (
    id integer NOT NULL,
    id_bat character varying(80),
    id_bat_lot character varying(80)
);


--
-- Name: c_batiment_bat_lot_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.c_batiment_bat_lot_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: c_batiment_bat_lot_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.c_batiment_bat_lot_id_seq OWNED BY relations.c_batiment_bat_lot.id;


--
-- Name: c_ens_immo_amgmt; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.c_ens_immo_amgmt (
    id integer NOT NULL,
    id_ens_immo character varying(80),
    id_amgmt character varying(80)
);


--
-- Name: c_ens_immo_amgmt_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.c_ens_immo_amgmt_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: c_ens_immo_amgmt_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.c_ens_immo_amgmt_id_seq OWNED BY relations.c_ens_immo_amgmt.id;


--
-- Name: c_ens_immo_bat; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.c_ens_immo_bat (
    id integer NOT NULL,
    id_ens_immo character varying(80),
    id_bat character varying(80)
);


--
-- Name: c_ens_immo_bat_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.c_ens_immo_bat_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: c_ens_immo_bat_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.c_ens_immo_bat_id_seq OWNED BY relations.c_ens_immo_bat.id;


--
-- Name: c_terrain_ens_immo; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.c_terrain_ens_immo (
    id integer NOT NULL,
    id_terrain character varying(80),
    id_ens_immo character varying(80)
);


--
-- Name: c_terrain_ens_immo_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.c_terrain_ens_immo_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: c_terrain_ens_immo_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.c_terrain_ens_immo_id_seq OWNED BY relations.c_terrain_ens_immo.id;


--
-- Name: t_actes; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_actes (
    cnumero character varying(20) NOT NULL,
    nature text,
    numero_avenant text,
    description text,
    text_de_deliberation text,
    text_signature text,
    en_attente text,
    amodiataire text,
    tiers_ac_amodiataire text,
    exploitant text,
    tiers_ac_exploitant text,
    bien text,
    text_bien text,
    bien_en_option text,
    text_bien_option text,
    periodicite_facture text,
    base_ text,
    duree_annees text,
    duree_mois text,
    duree_jours text,
    text_d_effet text,
    text_fin_periode_ini text,
    preavis_fin_initiale text,
    nbre_de_renouvelle text,
    duree_renouvellement text,
    unite_renouvellement text,
    preavis_en_mois_reno text,
    redevance_variable text,
    abattement_multiple text,
    echeance_30_jours_fd text,
    n_d_immeuble text,
    regime_de_tva_ text,
    text_origine text,
    operation_origine text,
    explication_origine text,
    acte_origine_ text,
    text_destination text,
    operation_destinatio text,
    explication_destinat text,
    acte_destination text,
    nombre_facturations text,
    nombre_sous_locataire text,
    presence_agenda text,
    presence_particulari text,
    numero_derniere_fact text,
    text_derniere_factur text,
    type_derniere_factur text,
    utilisateur text,
    text_acte text,
    heure_acte text,
    repertoires_des_anne text,
    faculte_resiliation text,
    nbannee_resiliation text,
    nbmois_resiliation text,
    nbjour_resiliation text,
    text_faculte_resiliation text,
    text_exercice_resiliation text,
    renouvellement text,
    rrr_redevance text,
    rrr_traffic text,
    remarque_renouvellement text,
    text_fin_contrat_avec_renouv text,
    id_udc text,
    id_activite text,
    observation text,
    tacite_reconduction text
);


--
-- Name: t_adresse; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_adresse (
    id bigint NOT NULL,
    id_adresse character varying(80),
    numero_rue character varying(80),
    rue character varying(80),
    code_postal character varying(80),
    commune character varying(80),
    code_insee_commune character varying(80),
    code_fantoir character varying(80)
);


--
-- Name: t_adresse_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_adresse_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_adresse_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_adresse_id_seq OWNED BY relations.t_adresse.id;


--
-- Name: t_amgmt; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_amgmt (
    id integer NOT NULL,
    id_amgmt character varying(80),
    fk_adresse character varying(80),
    description character varying(80)
);


--
-- Name: t_amgmt_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_amgmt_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_amgmt_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_amgmt_id_seq OWNED BY relations.t_amgmt.id;


--
-- Name: t_amgmt_lot; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_amgmt_lot (
    id integer NOT NULL,
    geom public.geometry(MultiPolygon,3948),
    id_amgmt_lot character varying(80),
    fk_adresse character varying(80),
    numero character varying(80),
    type character varying(80),
    etat character varying(80),
    description character varying(80),
    fk_acte character varying(80),
    fk_invariant character varying(80)
);


--
-- Name: t_amgmt_lot_xy_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_amgmt_lot_xy_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_amgmt_lot_xy_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_amgmt_lot_xy_id_seq OWNED BY relations.t_amgmt_lot.id;


--
-- Name: t_bat; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_bat (
    id integer NOT NULL,
    geom public.geometry(MultiPolygon,3948),
    id_bat character varying(80),
    fk_adresse character varying(80),
    descriptio character varying(80),
    apercu text,
    repertoire text,
    date_couverture date,
    date_ravalement date
);


--
-- Name: t_bat_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_bat_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_bat_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_bat_id_seq OWNED BY relations.t_bat.id;


--
-- Name: t_bat_lot; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_bat_lot (
    id integer NOT NULL,
    id_bat_lot character varying(80),
    fk_adresse character varying(80),
    num_lot character varying(80),
    type character varying(80),
    etat character varying(80),
    description character varying(80),
    fk_acte character varying(80),
    repertoire text,
    fk_invariant character varying(80)
);


--
-- Name: t_bat_lot_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_bat_lot_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_bat_lot_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_bat_lot_id_seq OWNED BY relations.t_bat_lot.id;


--
-- Name: t_ens_immo; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_ens_immo (
    id integer NOT NULL,
    id_ens_immo character varying(80),
    fk_adresse character varying(80),
    description character varying(80)
);


--
-- Name: t_ens_immo_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_ens_immo_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_ens_immo_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_ens_immo_id_seq OWNED BY relations.t_ens_immo.id;


--
-- Name: t_terrain; Type: TABLE; Schema: relations; Owner: -
--

CREATE TABLE relations.t_terrain (
    id integer NOT NULL,
    geom public.geometry(MultiPolygon,3948),
    id_terrain character varying(80),
    fk_adresse character varying(80),
    fk_acte character varying(80),
    nature character varying(80),
    fk_invariant character varying(80)
);


--
-- Name: t_terrain_xy_id_seq; Type: SEQUENCE; Schema: relations; Owner: -
--

CREATE SEQUENCE relations.t_terrain_xy_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: t_terrain_xy_id_seq; Type: SEQUENCE OWNED BY; Schema: relations; Owner: -
--

ALTER SEQUENCE relations.t_terrain_xy_id_seq OWNED BY relations.t_terrain.id;


--
-- Name: c_amgmt_amgmt_lot id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_amgmt_amgmt_lot ALTER COLUMN id SET DEFAULT nextval('relations.c_amgmt_amgmt_lot_id_seq'::regclass);


--
-- Name: c_batiment_bat_lot id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_batiment_bat_lot ALTER COLUMN id SET DEFAULT nextval('relations.c_batiment_bat_lot_id_seq'::regclass);


--
-- Name: c_ens_immo_amgmt id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_amgmt ALTER COLUMN id SET DEFAULT nextval('relations.c_ens_immo_amgmt_id_seq'::regclass);


--
-- Name: c_ens_immo_bat id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_bat ALTER COLUMN id SET DEFAULT nextval('relations.c_ens_immo_bat_id_seq'::regclass);


--
-- Name: c_terrain_ens_immo id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_terrain_ens_immo ALTER COLUMN id SET DEFAULT nextval('relations.c_terrain_ens_immo_id_seq'::regclass);


--
-- Name: t_adresse id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_adresse ALTER COLUMN id SET DEFAULT nextval('relations.t_adresse_id_seq'::regclass);


--
-- Name: t_amgmt id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_amgmt ALTER COLUMN id SET DEFAULT nextval('relations.t_amgmt_id_seq'::regclass);


--
-- Name: t_amgmt_lot id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_amgmt_lot ALTER COLUMN id SET DEFAULT nextval('relations.t_amgmt_lot_xy_id_seq'::regclass);


--
-- Name: t_bat id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat ALTER COLUMN id SET DEFAULT nextval('relations.t_bat_id_seq'::regclass);


--
-- Name: t_bat_lot id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat_lot ALTER COLUMN id SET DEFAULT nextval('relations.t_bat_lot_id_seq'::regclass);


--
-- Name: t_ens_immo id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_ens_immo ALTER COLUMN id SET DEFAULT nextval('relations.t_ens_immo_id_seq'::regclass);


--
-- Name: t_terrain id; Type: DEFAULT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_terrain ALTER COLUMN id SET DEFAULT nextval('relations.t_terrain_xy_id_seq'::regclass);


--
-- Name: t_amgmt ID_AMGMT; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_amgmt
    ADD CONSTRAINT "ID_AMGMT" PRIMARY KEY (id);


--
-- Name: c_amgmt_amgmt_lot c_amgmt_amgmt_lot_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_amgmt_amgmt_lot
    ADD CONSTRAINT c_amgmt_amgmt_lot_pkey PRIMARY KEY (id);


--
-- Name: c_batiment_bat_lot c_batiment_bat_lot_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_batiment_bat_lot
    ADD CONSTRAINT c_batiment_bat_lot_pkey PRIMARY KEY (id);


--
-- Name: c_ens_immo_amgmt c_ens_immo_amgmt_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_amgmt
    ADD CONSTRAINT c_ens_immo_amgmt_pkey PRIMARY KEY (id);


--
-- Name: c_ens_immo_bat c_ens_immo_bat_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_bat
    ADD CONSTRAINT c_ens_immo_bat_pkey PRIMARY KEY (id);


--
-- Name: t_actes id_numero; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_actes
    ADD CONSTRAINT id_numero PRIMARY KEY (cnumero);


--
-- Name: t_adresse t_adresse_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_adresse
    ADD CONSTRAINT t_adresse_pkey PRIMARY KEY (id);


--
-- Name: t_bat t_bat_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat
    ADD CONSTRAINT t_bat_pkey PRIMARY KEY (id);


--
-- Name: t_ens_immo t_ens_immo_pkey; Type: CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_ens_immo
    ADD CONSTRAINT t_ens_immo_pkey PRIMARY KEY (id);


--
-- Name: c_ens_immo_amgmt_id_amgmt_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX c_ens_immo_amgmt_id_amgmt_key ON relations.c_ens_immo_amgmt USING btree (id_amgmt);


--
-- Name: c_ens_immo_bat_id_bat_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX c_ens_immo_bat_id_bat_key ON relations.c_ens_immo_bat USING btree (id_bat);


--
-- Name: sidx_t_bat_geom; Type: INDEX; Schema: relations; Owner: -
--

CREATE INDEX sidx_t_bat_geom ON relations.t_bat USING gist (geom);


--
-- Name: t_adresse_id_adresse_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_adresse_id_adresse_key ON relations.t_adresse USING btree (id_adresse);


--
-- Name: t_amgmt_id_amgmt_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_amgmt_id_amgmt_key ON relations.t_amgmt USING btree (id_amgmt);


--
-- Name: t_amgmt_lot_id_amgmt_lot_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_amgmt_lot_id_amgmt_lot_key ON relations.t_amgmt_lot USING btree (id_amgmt_lot);


--
-- Name: t_bat_id_bat_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_bat_id_bat_key ON relations.t_bat USING btree (id_bat);


--
-- Name: t_bat_lot_id_bat_lot_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_bat_lot_id_bat_lot_key ON relations.t_bat_lot USING btree (id_bat_lot);


--
-- Name: t_ens_immo_id_ens_immo_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_ens_immo_id_ens_immo_key ON relations.t_ens_immo USING btree (id_ens_immo);


--
-- Name: t_terrain_id_terrain_key; Type: INDEX; Schema: relations; Owner: -
--

CREATE UNIQUE INDEX t_terrain_id_terrain_key ON relations.t_terrain USING btree (id_terrain);


--
-- Name: c_amgmt_amgmt_lot c_amgmt_amgmt_lot_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_amgmt_amgmt_lot
    ADD CONSTRAINT c_amgmt_amgmt_lot_fk FOREIGN KEY (id_amgmt) REFERENCES relations.t_amgmt(id_amgmt);


--
-- Name: c_amgmt_amgmt_lot c_amgmt_amgmt_lot_fk_amgt_lot; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_amgmt_amgmt_lot
    ADD CONSTRAINT c_amgmt_amgmt_lot_fk_amgt_lot FOREIGN KEY (id_amgmt_lot) REFERENCES relations.t_amgmt_lot(id_amgmt_lot);


--
-- Name: c_batiment_bat_lot c_batiment_bat_lot_fk_bat_lot; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_batiment_bat_lot
    ADD CONSTRAINT c_batiment_bat_lot_fk_bat_lot FOREIGN KEY (id_bat_lot) REFERENCES relations.t_bat_lot(id_bat_lot);


--
-- Name: c_batiment_bat_lot c_batiment_bat_lot_fk_t_bat; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_batiment_bat_lot
    ADD CONSTRAINT c_batiment_bat_lot_fk_t_bat FOREIGN KEY (id_bat) REFERENCES relations.t_bat(id_bat);


--
-- Name: c_ens_immo_amgmt c_ens_immo_amgmt_fk_amgmt; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_amgmt
    ADD CONSTRAINT c_ens_immo_amgmt_fk_amgmt FOREIGN KEY (id_amgmt) REFERENCES relations.t_amgmt(id_amgmt);


--
-- Name: c_ens_immo_amgmt c_ens_immo_amgmt_fk_ens_immo; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_amgmt
    ADD CONSTRAINT c_ens_immo_amgmt_fk_ens_immo FOREIGN KEY (id_ens_immo) REFERENCES relations.t_ens_immo(id_ens_immo);


--
-- Name: c_ens_immo_bat c_ens_immo_bat_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_ens_immo_bat
    ADD CONSTRAINT c_ens_immo_bat_fk FOREIGN KEY (id_ens_immo) REFERENCES relations.t_ens_immo(id_ens_immo);


--
-- Name: c_terrain_ens_immo c_terrain_ens_immo_fk_id_ens_immo; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_terrain_ens_immo
    ADD CONSTRAINT c_terrain_ens_immo_fk_id_ens_immo FOREIGN KEY (id_ens_immo) REFERENCES relations.t_ens_immo(id_ens_immo);


--
-- Name: c_terrain_ens_immo c_terrain_ens_immo_fk_id_terrain; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.c_terrain_ens_immo
    ADD CONSTRAINT c_terrain_ens_immo_fk_id_terrain FOREIGN KEY (id_terrain) REFERENCES relations.t_terrain(id_terrain);


--
-- Name: t_amgmt_lot t_amgmt_lot_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_amgmt_lot
    ADD CONSTRAINT t_amgmt_lot_fk FOREIGN KEY (fk_adresse) REFERENCES relations.t_adresse(id_adresse);


--
-- Name: t_amgmt_lot t_amgmt_lot_fk1; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_amgmt_lot
    ADD CONSTRAINT t_amgmt_lot_fk1 FOREIGN KEY (fk_acte) REFERENCES relations.t_actes(cnumero) MATCH FULL;


--
-- Name: t_bat t_bat_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat
    ADD CONSTRAINT t_bat_fk FOREIGN KEY (id_bat) REFERENCES relations.c_ens_immo_bat(id_bat);


--
-- Name: t_bat t_bat_fk_adresse; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat
    ADD CONSTRAINT t_bat_fk_adresse FOREIGN KEY (fk_adresse) REFERENCES relations.t_adresse(id_adresse);


--
-- Name: t_bat_lot t_bat_lot_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat_lot
    ADD CONSTRAINT t_bat_lot_fk FOREIGN KEY (fk_adresse) REFERENCES relations.t_adresse(id_adresse);


--
-- Name: t_bat_lot t_bat_lot_fk_acte; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_bat_lot
    ADD CONSTRAINT t_bat_lot_fk_acte FOREIGN KEY (fk_acte) REFERENCES relations.t_actes(cnumero);


--
-- Name: t_ens_immo t_ens_immo_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_ens_immo
    ADD CONSTRAINT t_ens_immo_fk FOREIGN KEY (fk_adresse) REFERENCES relations.t_adresse(id_adresse);


--
-- Name: t_terrain t_terrain_fk; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_terrain
    ADD CONSTRAINT t_terrain_fk FOREIGN KEY (fk_adresse) REFERENCES relations.t_adresse(id_adresse);


--
-- Name: t_terrain t_terrain_fk1; Type: FK CONSTRAINT; Schema: relations; Owner: -
--

ALTER TABLE ONLY relations.t_terrain
    ADD CONSTRAINT t_terrain_fk1 FOREIGN KEY (fk_acte) REFERENCES relations.t_actes(cnumero);



--
-- Spaced schema and table
--

ALTER TABLE IF EXISTS ONLY "spaced schema"."spaced child" DROP CONSTRAINT IF EXISTS parent_fk;
ALTER TABLE IF EXISTS ONLY "spaced schema"."spaced parent" DROP CONSTRAINT IF EXISTS "spaced parent_pkey";
ALTER TABLE IF EXISTS ONLY "spaced schema"."spaced child" DROP CONSTRAINT IF EXISTS "spaced child_pkey";
ALTER TABLE IF EXISTS "spaced schema"."spaced parent" ALTER COLUMN id DROP DEFAULT;
ALTER TABLE IF EXISTS "spaced schema"."spaced child" ALTER COLUMN id DROP DEFAULT;
DROP SEQUENCE IF EXISTS "spaced schema"."spaced parent_id_seq";
DROP TABLE IF EXISTS "spaced schema"."spaced parent";
DROP SEQUENCE IF EXISTS "spaced schema"."spaced child_id_seq";
DROP TABLE IF EXISTS "spaced schema"."spaced child";
DROP SCHEMA IF EXISTS "spaced schema";
--
-- Name: spaced schema; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA "spaced schema";


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: spaced child; Type: TABLE; Schema: spaced schema; Owner: -
--

CREATE TABLE "spaced schema"."spaced child" (
    id integer NOT NULL,
    name character varying,
    parent_id integer
);


--
-- Name: spaced child_id_seq; Type: SEQUENCE; Schema: spaced schema; Owner: -
--

CREATE SEQUENCE "spaced schema"."spaced child_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: spaced child_id_seq; Type: SEQUENCE OWNED BY; Schema: spaced schema; Owner: -
--

ALTER SEQUENCE "spaced schema"."spaced child_id_seq" OWNED BY "spaced schema"."spaced child".id;


--
-- Name: spaced parent; Type: TABLE; Schema: spaced schema; Owner: -
--

CREATE TABLE "spaced schema"."spaced parent" (
    id integer NOT NULL,
    name character varying
);


--
-- Name: spaced parent_id_seq; Type: SEQUENCE; Schema: spaced schema; Owner: -
--

CREATE SEQUENCE "spaced schema"."spaced parent_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: spaced parent_id_seq; Type: SEQUENCE OWNED BY; Schema: spaced schema; Owner: -
--

ALTER SEQUENCE "spaced schema"."spaced parent_id_seq" OWNED BY "spaced schema"."spaced parent".id;


--
-- Name: spaced child id; Type: DEFAULT; Schema: spaced schema; Owner: -
--

ALTER TABLE ONLY "spaced schema"."spaced child" ALTER COLUMN id SET DEFAULT nextval('"spaced schema"."spaced child_id_seq"'::regclass);


--
-- Name: spaced parent id; Type: DEFAULT; Schema: spaced schema; Owner: -
--

ALTER TABLE ONLY "spaced schema"."spaced parent" ALTER COLUMN id SET DEFAULT nextval('"spaced schema"."spaced parent_id_seq"'::regclass);


--
-- Name: spaced child spaced child_pkey; Type: CONSTRAINT; Schema: spaced schema; Owner: -
--

ALTER TABLE ONLY "spaced schema"."spaced child"
    ADD CONSTRAINT "spaced child_pkey" PRIMARY KEY (id);


--
-- Name: spaced parent spaced parent_pkey; Type: CONSTRAINT; Schema: spaced schema; Owner: -
--

ALTER TABLE ONLY "spaced schema"."spaced parent"
    ADD CONSTRAINT "spaced parent_pkey" PRIMARY KEY (id);


--
-- Name: spaced child parent_fk; Type: FK CONSTRAINT; Schema: spaced schema; Owner: -
--

ALTER TABLE ONLY "spaced schema"."spaced child"
    ADD CONSTRAINT parent_fk FOREIGN KEY (parent_id) REFERENCES "spaced schema"."spaced parent"(id);

