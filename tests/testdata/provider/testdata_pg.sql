--
-- PostgreSQL database dump
--

-- Dumped from database version 9.3.6
-- Dumped by pg_dump version 9.3.6
-- Started on 2015-05-21 09:37:33 CEST

SET statement_timeout = 0;
-- SET lock_timeout = 0;
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
    name2 text DEFAULT 'qgis',
    num_char text,
    geom public.geometry(Point,4326)
);

CREATE TABLE qgis_test."some_poly_data" (
    pk SERIAL NOT NULL,
    geom public.geometry(Polygon,4326)
);

--
-- TOC entry 4068 (class 0 OID 377761)
-- Dependencies: 171
-- Data for Name: someData; Type: TABLE DATA; Schema: qgis_test; Owner: postgres
--

INSERT INTO qgis_test."someData" (pk, cnt, name, name2, num_char, geom) VALUES
(5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(3,  300, 'Pear', 'PEaR', '3', NULL),
(1,  100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(2,  200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(4,  400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')
;

INSERT INTO qgis_test."some_poly_data" (pk, geom) VALUES
(1, ST_GeomFromText('Polygon ((-69.0 81.4, -69.0 80.2, -73.7 80.2, -73.7 76.3, -74.9 76.3, -74.9 81.4, -69.0 81.4))', 4326) ),
(2, ST_GeomFromText('Polygon ((-67.6 81.2, -66.3 81.2, -66.3 76.9, -67.6 76.9, -67.6 81.2))', 4326) ),
(3, ST_GeomFromText('Polygon ((-68.4 75.8, -67.5 72.6, -68.6 73.7, -70.2 72.9, -68.4 75.8))', 4326) ),
(4, NULL)
;

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

CREATE TABLE qgis_test.date_times(
       id int,
       date_field date,
       time_field time,
       datetime_field timestamp without time zone
);

INSERT INTO qgis_test.date_times values (1, '2004-03-04'::date, '13:41:52'::time, '2004-03-04 13:41:52'::timestamp without time zone );

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


-----------------------------------------
-- Test tables with INHERITS
--
-- This is bad design: the common fields
-- are replicated in child tables and
-- leads to duplicated ids in the parent
-- table
--


CREATE TABLE qgis_test.base_table_bad
(
  gid serial NOT NULL,
  geom geometry(Point,4326),
  code character varying,
  CONSTRAINT base_bad_pkey PRIMARY KEY (gid)
)
WITH (
  OIDS=FALSE
);

CREATE TABLE qgis_test.child_table_bad
(
  gid serial NOT NULL,
  geom geometry(Point,4326),
  code character varying,
  CONSTRAINT child_bad_pkey PRIMARY KEY (gid)
)
INHERITS ( qgis_test.base_table_bad)
WITH (
  OIDS=FALSE
);


CREATE TABLE qgis_test.child_table2_bad
(
  gid serial NOT NULL,
  geom geometry(Point,4326),
  code character varying,
  CONSTRAINT child2_bad_pkey PRIMARY KEY (gid)
)
INHERITS ( qgis_test.base_table_bad)
WITH (
  OIDS=FALSE
);

INSERT INTO qgis_test.child_table_bad (geom, code) VALUES ('srid=4326;Point(0 0)'::geometry, 'child 1');
INSERT INTO qgis_test.child_table_bad (geom, code) VALUES ('srid=4326;Point(1 1)'::geometry, 'child 2');


INSERT INTO qgis_test.child_table2_bad (geom, code) VALUES ('srid=4326;Point(-1 -1)'::geometry, 'child2 1');
INSERT INTO qgis_test.child_table2_bad (geom, code) VALUES ('srid=4326;Point(-1 1)'::geometry, 'child2 2');



-----------------------------------------
-- Test tables with INHERITS
--
-- This is good design: the common fields
-- and the pk are only in the parent table
-- no pk duplication


CREATE TABLE qgis_test.base_table_good
(
  gid serial NOT NULL,
  geom geometry(Point,4326),
  CONSTRAINT base_good_pkey PRIMARY KEY (gid)
)
WITH (
  OIDS=FALSE
);

CREATE TABLE qgis_test.child_table_good
(
  code1 character varying
)
INHERITS ( qgis_test.base_table_good)
WITH (
  OIDS=FALSE
);


CREATE TABLE qgis_test.child_table2_good
(
  code2 character varying
)
INHERITS ( qgis_test.base_table_good)
WITH (
  OIDS=FALSE
);

INSERT INTO qgis_test.child_table_good (geom, code1) VALUES ('srid=4326;Point(0 0)'::geometry, 'child 1');
INSERT INTO qgis_test.child_table_good (geom, code1) VALUES ('srid=4326;Point(1 1)'::geometry, 'child 2');


INSERT INTO qgis_test.child_table2_good (geom, code2) VALUES ('srid=4326;Point(-1 -1)'::geometry, 'child2 1');
INSERT INTO qgis_test.child_table2_good (geom, code2) VALUES ('srid=4326;Point(-1 1)'::geometry, 'child2 2');

--------------------------------------
-- A writable view with an int pk
--

CREATE TABLE qgis_test.bikes
(
  pk serial NOT NULL,
  name character varying(255)
);

CREATE OR REPLACE VIEW qgis_test.bikes_view
  AS
    SELECT *
    FROM qgis_test.bikes;

CREATE OR REPLACE FUNCTION qgis_test.bikes_view_insert()
  RETURNS trigger AS
$BODY$
BEGIN
  INSERT INTO qgis_test.bikes (
    "name"
  )
  VALUES (
    NEW.name
  )
  RETURNING pk INTO NEW.pk;

  RETURN NEW;
END; $BODY$
  LANGUAGE plpgsql VOLATILE;

CREATE TRIGGER bikes_view_ON_INSERT INSTEAD OF INSERT ON qgis_test.bikes_view
  FOR EACH ROW EXECUTE PROCEDURE qgis_test.bikes_view_insert();

--------------------------------------
-- A string primary key to force usage of pktMap
--

CREATE SEQUENCE qgis_test.oid_serial;

CREATE TABLE qgis_test.oid_serial_table
(
  obj_id varchar(16) NOT NULL,
  name character varying(255),
  CONSTRAINT pkey_oid_serial PRIMARY KEY (obj_id)
);

ALTER TABLE qgis_test.oid_serial_table ALTER COLUMN obj_id SET DEFAULT 'prf_' || nextval('qgis_test.oid_serial');


--------------------------------------
-- Test use of domain types
--

CREATE DOMAIN qgis_test.var_char_domain
  AS character varying;

CREATE DOMAIN qgis_test.var_char_domain_6
  AS character varying(6);

CREATE DOMAIN qgis_test.character_domain
  AS character;

CREATE DOMAIN qgis_test.character_domain_6
  AS character(6);

CREATE DOMAIN qgis_test.char_domain
  AS char;

CREATE DOMAIN qgis_test.char_domain_6
  AS char(6);

CREATE DOMAIN qgis_test.text_domain
  AS text;

CREATE DOMAIN qgis_test.numeric_domain
  AS numeric(10,4);

CREATE TABLE qgis_test.domains
(
  fld_var_char_domain qgis_test.var_char_domain,
  fld_var_char_domain_6 qgis_test.var_char_domain_6,
  fld_character_domain qgis_test.character_domain,
  fld_character_domain_6 qgis_test.character_domain_6,
  fld_char_domain qgis_test.char_domain,
  fld_char_domain_6 qgis_test.char_domain_6,
  fld_text_domain qgis_test.text_domain,
  fld_numeric_domain qgis_test.numeric_domain
);


--------------------------------------
-- Temporary table for testing renaming fields
--

CREATE TABLE qgis_test.rename_table
(
  gid serial NOT NULL,
  field1 text,
  field2 text
);

INSERT INTO qgis_test.rename_table (field1,field2) VALUES ('a','b');
