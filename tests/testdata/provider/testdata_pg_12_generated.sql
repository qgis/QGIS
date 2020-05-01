-- valid only for PostgreSQL 12 and newer, because we are testing features
-- available only on this version.

CREATE TABLE qgis_test.test_gen_col (
  id SERIAL PRIMARY KEY,
  geom GEOMETRY('Polygon', 4326) NOT NULL,
  cent GEOMETRY('Point') GENERATED ALWAYS AS ( st_centroid(geom) ) STORED
);
