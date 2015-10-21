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

CREATE EXTENSION IF NOT EXISTS postgis;

DROP SCHEMA IF EXISTS qgis_test CASCADE;
CREATE SCHEMA qgis_test;


SET default_tablespace = '';

SET default_with_oids = false;

--
-- TOC entry 171 (class 1259 OID 377761)
-- Name: someData; Type: TABLE; Schema: qgis_test; Owner: postgres; Tablespace: 
--

CREATE TABLE qgis_test."someData" (
    pk SERIAL NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    geom public.geometry(Point,4326)
);


--
-- TOC entry 4068 (class 0 OID 377761)
-- Dependencies: 171
-- Data for Name: someData; Type: TABLE DATA; Schema: qgis_test; Owner: postgres
--

COPY qgis_test."someData" (pk, cnt, name, geom) FROM stdin;
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

ALTER TABLE ONLY qgis_test."someData"
    ADD CONSTRAINT "someData_pkey" PRIMARY KEY (pk);


-- Completed on 2015-05-21 09:37:33 CEST

--
-- PostgreSQL database dump complete
--

CREATE TABLE qgis_test.p2d(
       id int,
       geom Geometry(Polygon,4326)
);
INSERT INTO qgis_test.p2d values (1, 'srid=4326;Polygon((0 0,1 0,1 1,0 1,0 0))'::geometry);

CREATE TABLE qgis_test.p3d(
       id int,
       geom Geometry(PolygonZ,4326)
);
INSERT INTO qgis_test.p3d values (1, 'srid=4326;Polygon((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0))'::geometry);

CREATE TABLE qgis_test.triangle2d(
       id int,
       geom Geometry(Triangle,4326)
);

INSERT INTO qgis_test.triangle2d values (1, 'srid=4326;triangle((0 0,1 0,1 1,0 0))'::geometry);

CREATE TABLE qgis_test.triangle3d(
       id int,
       geom Geometry(TriangleZ,4326)
);

INSERT INTO qgis_test.triangle3d values (1, 'srid=4326;triangle((0 0 0,1 0 0,1 1 0,0 0 0))'::geometry);

CREATE TABLE qgis_test.tin2d(
       id int,
       geom Geometry(TIN,4326)
);

INSERT INTO qgis_test.tin2d values (1, 'srid=4326;TIN(((0 0,1 0,1 1,0 0)),((0 0,0 1,1 1,0 0)))'::geometry);

CREATE TABLE qgis_test.tin3d(
       id int,
       geom Geometry(TINZ,4326)
);

INSERT INTO qgis_test.tin3d values (1, 'srid=4326;TIN(((0 0 0,1 0 0,1 1 0,0 0 0)),((0 0 0,0 1 0,1 1 0,0 0 0)))'::geometry);

CREATE TABLE qgis_test.ps2d(
       id int,
       geom Geometry(PolyhedralSurface,4326)
);

INSERT INTO qgis_test.ps2d values (1, 'srid=4326;PolyhedralSurface(((0 0,1 0,1 1,0 1,0 0)))'::geometry);

CREATE TABLE qgis_test.ps3d(
       id int,
       geom Geometry(PolyhedralSurfaceZ,4326)
);

INSERT INTO qgis_test.ps3d values (1, 'srid=4326;PolyhedralSurface Z(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 1,1 0 1,1 1 1,0 1 1,0 0 1)),((0 0 0,0 0 1,0 1 1,0 1 0,0 0 0)),((0 1 0,0 1 1,1 1 1,1 1 0,0 1 0)),((1 1 0,1 1 1,1 0 1,1 0 0,1 1 0)),((1 0 0,1 0 1,0 0 1,0 0 0,1 0 0)))'::geometry);

CREATE TABLE qgis_test.mp3d(
       id int,
       geom Geometry(MultipolygonZ,4326)
);

INSERT INTO qgis_test.mp3d values (1, 'srid=4326;Multipolygon Z(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 1,1 0 1,1 1 1,0 1 1,0 0 1)),((0 0 0,0 0 1,0 1 1,0 1 0,0 0 0)),((0 1 0,0 1 1,1 1 1,1 1 0,0 1 0)),((1 1 0,1 1 1,1 0 1,1 0 0,1 1 0)),((1 0 0,1 0 1,0 0 1,0 0 0,1 0 0)))'::geometry);

CREATE TABLE qgis_test.pt2d(
       id int,
       geom Geometry(Point,4326)
);

INSERT INTO qgis_test.pt2d values (1, 'srid=4326;Point(0 0)'::geometry);

CREATE TABLE qgis_test.pt3d(
       id int,
       geom Geometry(PointZ,4326)
);

INSERT INTO qgis_test.pt3d values (1, 'srid=4326;PointZ(0 0 0)'::geometry);

CREATE TABLE qgis_test.ls2d(
       id int,
       geom Geometry(LineString,4326)
);

INSERT INTO qgis_test.ls2d values (1, 'srid=4326;Linestring(0 0, 1 1)'::geometry);

CREATE TABLE qgis_test.ls3d(
       id int,
       geom Geometry(LineStringZ,4326)
);

INSERT INTO qgis_test.ls3d values (1, 'srid=4326;Linestring(0 0 0, 1 1 1)'::geometry);

CREATE TABLE qgis_test.mpt2d(
       id int,
       geom Geometry(MultiPoint,4326)
);

INSERT INTO qgis_test.mpt2d values (1, 'srid=4326;MultiPoint((0 0),(1 1))'::geometry);

CREATE TABLE qgis_test.mpt3d(
       id int,
       geom Geometry(MultiPointZ,4326)
);

INSERT INTO qgis_test.mpt3d values (1, 'srid=4326;MultiPoint((0 0 0),(1 1 1))'::geometry);

CREATE TABLE qgis_test.mls2d(
       id int,
       geom Geometry(MultiLineString,4326)
);

INSERT INTO qgis_test.mls2d values (1, 'srid=4326;MultiLineString((0 0, 1 1),(2 2, 3 3))'::geometry);

CREATE TABLE qgis_test.mls3d(
       id int,
       geom Geometry(MultiLineStringZ,4326)
);

INSERT INTO qgis_test.mls3d values (1, 'srid=4326;MultiLineString((0 0 0, 1 1 1),(2 2 2, 3 3 3))'::geometry);
