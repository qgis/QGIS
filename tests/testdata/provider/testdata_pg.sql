
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
CREATE EXTENSION IF NOT EXISTS citext;


--- Create qgis_test schema
DROP SCHEMA IF EXISTS qgis_test CASCADE;
CREATE SCHEMA qgis_test;
GRANT ALL ON SCHEMA qgis_test TO public;
ALTER DEFAULT PRIVILEGES IN SCHEMA qgis_test GRANT ALL ON TABLES TO public;
ALTER DEFAULT PRIVILEGES IN SCHEMA qgis_test GRANT ALL ON SEQUENCES TO public;


--- Create "CamelCaseSchema" schema
DROP SCHEMA IF EXISTS "CamelCaseSchema" CASCADE;
CREATE SCHEMA "CamelCaseSchema";
GRANT ALL ON SCHEMA "CamelCaseSchema" TO public;
ALTER DEFAULT PRIVILEGES IN SCHEMA "CamelCaseSchema" GRANT ALL ON TABLES TO public;
ALTER DEFAULT PRIVILEGES IN SCHEMA "CamelCaseSchema" GRANT ALL ON SEQUENCES TO public;


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

COMMENT ON TABLE qgis_test."someData" IS 'QGIS Test Table';

CREATE TABLE qgis_test."some_poly_data" (
    pk SERIAL NOT NULL,
    geom public.geometry(Polygon,4326)
);

CREATE OR REPLACE VIEW qgis_test.some_poly_data_view
  AS
    SELECT *
    FROM qgis_test.some_poly_data;

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


CREATE TABLE qgis_test.array_tbl (id serial PRIMARY KEY, location int[], geom geometry(Point,3857));

INSERT INTO qgis_test.array_tbl (location, geom) VALUES ('{1, 2, 3}', 'srid=3857;Point(913209.0358 5606025.2373)'::geometry);
INSERT INTO qgis_test.array_tbl (location, geom) VALUES ('{}', 'srid=3857;Point(913214.6741 5606017.8743)'::geometry);
INSERT INTO qgis_test.array_tbl (geom) VALUES ('srid=3857;Point(913204.9128 5606011.4565)'::geometry);


-- Provider check with compound key

CREATE TABLE qgis_test."someDataCompound" (
    pk integer NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    name2 text DEFAULT 'qgis',
    num_char text,
    geom public.geometry(Point,4326),
    key1 integer,
    key2 integer,
    PRIMARY KEY(key1, key2)
);

INSERT INTO qgis_test."someDataCompound" ( key1, key2, pk, cnt, name, name2, num_char, geom) VALUES
(1, 1, 5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL),
(2, 1, 1,  100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(2, 2, 2,  200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(2, 3, 4,  400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')
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


-- Test of 4D geometries (with Z and M values)

CREATE TABLE qgis_test.pt4d(
       id int,
       geom Geometry(PointZM,4326)
);

INSERT INTO qgis_test.pt4d values (1, 'srid=4326;PointZM(1 2 3 4)'::geometry);



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


--------------------------------------
-- Table for editor widget types
--

DROP TABLE IF EXISTS qgis_editor_widget_styles;

CREATE TABLE qgis_editor_widget_styles
(
  schema_name TEXT NOT NULL,
  table_name TEXT NOT NULL,
  field_name TEXT NOT NULL,
  type TEXT NOT NULL,
  config TEXT,
  PRIMARY KEY(table_name, field_name)
);

CREATE TABLE qgis_test.widget_styles(
       id int PRIMARY KEY,
       fld1 TEXT,
       fld2 TEXT
);

INSERT INTO qgis_editor_widget_styles VALUES
('qgis_test', 'widget_styles', 'fld1', 'FooEdit', '<config type="Map"><Option name="param1" value="value1" type="QString"/><Option name="param2" value="2" type="QString"/></config>');

--------------------------------------
-- Table for boolean
--

CREATE TABLE qgis_test.boolean_table
(
  id int PRIMARY KEY,
  fld1 BOOLEAN
);

INSERT INTO qgis_test.boolean_table VALUES
(1, TRUE),
(2, FALSE),
(3, NULL);


--------------------------------------
-- Table for citext
--

CREATE TABLE qgis_test.citext_table
(
  id int PRIMARY KEY,
  fld1 citext
);

INSERT INTO qgis_test.citext_table VALUES
(1, 'test val'),
(2, NULL);


--------------------------------------
-- Table for bytea
--

CREATE TABLE qgis_test.byte_a_table
(
  id int PRIMARY KEY,
  fld1 bytea
);

INSERT INTO qgis_test.byte_a_table VALUES
(1, encode('binvalue', 'base64')::bytea),
(2, NULL);


-----------------------------
-- Table for constraint tests
--

DROP TABLE IF EXISTS qgis_test.constraints;
CREATE TABLE qgis_test.constraints
(
  gid serial NOT NULL PRIMARY KEY, -- implicit unique key
  val int, -- unique constraint
  name text NOT NULL, -- unique index
  description text,
  CONSTRAINT constraint_val UNIQUE (val),
  CONSTRAINT constraint_val2 UNIQUE (val) -- create double unique constraint for test
);

CREATE UNIQUE INDEX constraints_uniq
  ON qgis_test.constraints
  USING btree
  (name COLLATE pg_catalog."default"); -- unique index

CREATE TABLE qgis_test.check_constraints (
  id integer PRIMARY KEY,
  a integer,
  b integer, CHECK (a > b)
);
INSERT INTO qgis_test.check_constraints VALUES (
  1, -- id
  4, -- a
  3  -- b
);


---------------------------------------------
--
-- Table and view for tests on  checkPrimaryKeyUnicity
--

DROP TABLE IF EXISTS qgis_test.b21839_pk_unicity CASCADE;
CREATE TABLE qgis_test.b21839_pk_unicity
(
  pk serial NOT NULL,
  an_int integer NOT NULL,
  a_unique_int integer NOT NULL,
  geom geometry(Point),
  CONSTRAINT b21839_pk_unicity_pkey PRIMARY KEY (pk)
)
WITH (
  OIDS=FALSE
);


INSERT INTO qgis_test.b21839_pk_unicity(
            pk, an_int, a_unique_int , geom)
    VALUES (1, 1, 1, ST_GeomFromText('point( 1 1)'));


INSERT INTO qgis_test.b21839_pk_unicity(
            pk, an_int, a_unique_int, geom)
    VALUES (2, 1, 2, ST_GeomFromText('point( 1 3)'));



CREATE OR REPLACE VIEW qgis_test.b21839_pk_unicity_view AS
 SELECT b21839_pk_unicity.pk,
    b21839_pk_unicity.an_int,
    b21839_pk_unicity.a_unique_int,
    b21839_pk_unicity.geom
   FROM qgis_test.b21839_pk_unicity;



---------------------------------------------
--
-- Table and views for tests on QgsAbstractProviderConnection
--

CREATE TABLE qgis_test.geometries_table (name VARCHAR, geom GEOMETRY);

INSERT INTO qgis_test.geometries_table VALUES
  ('Point', 'POINT(0 0)'),
  ('Point4326', 'SRID=4326;POINT(7 45)'),
  ('Point3857', 'SRID=3857;POINT(100 100)'),
  ('Linestring', 'LINESTRING(0 0, 1 1, 2 1, 2 2)'),
  ('Polygon', 'POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))'),
  ('PolygonWithHole', 'POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1))'),
  ('Collection', 'GEOMETRYCOLLECTION(POINT(2 0),POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)))');

CREATE VIEW qgis_test.geometries_view AS (SELECT * FROM qgis_test.geometries_table);

CREATE TABLE qgis_test.geometryless_table (name VARCHAR, value INTEGER);

---------------------------------------------
--
-- View with separate bbox field
--

CREATE VIEW qgis_test.some_poly_data_shift_bbox AS
 SELECT pk,
        geom,
        ST_Translate(
          ST_Envelope(geom),
          ST_XMax(ST_Envelope(geom)) - ST_XMin(ST_Envelope(geom)),
          0.0
        ) AS shiftbox
   FROM qgis_test.some_poly_data;


---------------------------------------------
--
-- View with tid PK field
--

CREATE TABLE qgis_test.b31799_test_table AS (SELECT (ST_DumpPoints(ST_GeneratePoints(ST_Expand('SRID=4326;POINT(0 0)'::geometry,90),10))).geom, random());
CREATE VIEW qgis_test.b31799_test_view_ctid AS (SELECT ctid, geom, random() FROM qgis_test.b31799_test_table, pg_sleep(0.1));

---------------------------------------------
--
-- Geometryless view
-- See https://github.com/qgis/QGIS/issues/32523
--
CREATE VIEW qgis_test.b32523 AS
  SELECT pk, random()
  FROM qgis_test.some_poly_data;
