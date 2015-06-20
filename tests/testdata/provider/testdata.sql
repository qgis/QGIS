--
-- PostgreSQL database dump
--

-- Dumped from database version 9.3.6
-- Dumped by pg_dump version 9.3.6
-- Started on 2015-05-21 09:37:33 CEST

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- TOC entry 7 (class 2615 OID 377760)
-- Name: qgis_test; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA qgis_test;


ALTER SCHEMA qgis_test OWNER TO postgres;

SET search_path = qgis_test, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- TOC entry 171 (class 1259 OID 377761)
-- Name: someData; Type: TABLE; Schema: qgis_test; Owner: postgres; Tablespace: 
--

CREATE TABLE "someData" (
    pk SERIAL NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    geom public.geometry(Point,4326)
);


ALTER TABLE qgis_test."someData" OWNER TO postgres;

--
-- TOC entry 4068 (class 0 OID 377761)
-- Dependencies: 171
-- Data for Name: someData; Type: TABLE DATA; Schema: qgis_test; Owner: postgres
--

COPY "someData" (pk, cnt, name, geom) FROM stdin;
5	-200	\N	0101000020E61000001D5A643BDFC751C01F85EB51B88E5340
3	300	Pear	\N
1	100	Orange	0101000020E61000006891ED7C3F9551C085EB51B81E955040
2	200	Apple	0101000020E6100000CDCCCCCCCC0C51C03333333333B35140
4	400	Honey	0101000020E610000014AE47E17A5450C03333333333935340
\.


--
-- TOC entry 3953 (class 2606 OID 377768)
-- Name: someData_pkey; Type: CONSTRAINT; Schema: qgis_test; Owner: postgres; Tablespace: 
--

ALTER TABLE ONLY "someData"
    ADD CONSTRAINT "someData_pkey" PRIMARY KEY (pk);


-- Completed on 2015-05-21 09:37:33 CEST

--
-- PostgreSQL database dump complete
--

