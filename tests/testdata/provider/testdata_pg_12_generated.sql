-- valid only for PostgreSQL 12 and newer, because we are testing features
-- available only on this version.

CREATE TABLE qgis_test.test_gen_col (
  id SERIAL PRIMARY KEY,
  name varchar(32),
  geom GEOMETRY('Polygon', 4326) NOT NULL,
  cent GEOMETRY('Point', 4326) NOT NULL GENERATED ALWAYS AS ( st_centroid(geom) ) STORED,
  poly_area FLOAT NOT NULL GENERATED ALWAYS AS ( st_area(st_transform(geom, 31979)) ) STORED
);

CREATE TABLE qgis_test.test_gen_geog_col (
  id SERIAL PRIMARY KEY,
  geog GEOGRAPHY('Polygon', 4326) NOT NULL,
  cent GEOGRAPHY('Point', 4326) NOT NULL GENERATED ALWAYS AS ( st_centroid(geog) ) STORED,
  poly_area FLOAT NOT NULL GENERATED ALWAYS AS ( st_area(geog) ) STORED
);

CREATE TABLE qgis_test.generated_columns(
  pk INTEGER PRIMARY KEY,
  generated_field varchar(30) GENERATED ALWAYS AS ('test:' || "pk"::varchar) STORED);
INSERT INTO qgis_test.generated_columns ("pk") VALUES(1);
