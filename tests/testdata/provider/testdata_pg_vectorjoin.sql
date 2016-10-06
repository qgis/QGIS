
--CREATE SCHEMA IF NOT EXISTS qgis_test;


DROP TABLE IF EXISTS qgis_test.table_a;
DROP TABLE IF EXISTS qgis_test.table_b;
DROP TABLE IF EXISTS qgis_test.table_c;
DROP TABLE IF EXISTS qgis_test.table_x;

CREATE TABLE qgis_test.table_a
(
  id_a integer NOT NULL,
  CONSTRAINT table_a_pkey PRIMARY KEY (id_a)
);

CREATE TABLE qgis_test.table_b
(
  id_b integer NOT NULL,
  value_b integer,
  CONSTRAINT table_b_pkey PRIMARY KEY (id_b)
);

CREATE TABLE qgis_test.table_c
(
  id_c integer NOT NULL,
  value_c integer,
  CONSTRAINT table_c_pkey PRIMARY KEY (id_c)
);

CREATE TABLE qgis_test.table_x
(
  id_x integer NOT NULL,
  value_x1 integer,
  value_x2 integer,
  CONSTRAINT table_x_pkey PRIMARY KEY (id_x)
);

