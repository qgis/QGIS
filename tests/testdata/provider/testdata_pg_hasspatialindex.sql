--CREATE SCHEMA IF NOT EXISTS qgis_test;


DROP TABLE IF EXISTS qgis_test.hspi_table;

CREATE TABLE qgis_test.hspi_table
(
  id serial PRIMARY KEY,
  geom_without_index geometry(Polygon,4326),
  geom_with_index geometry(Polygon,4326)
);
CREATE INDEX hspi_index_1 ON qgis_test.hspi_table USING GIST (geom_with_index);

CREATE MATERIALIZED view qgis_test.hspi_materialized_view AS SELECT * FROM qgis_test.hspi_table;
CREATE INDEX hspi_index_2 ON qgis_test.hspi_materialized_view USING GIST (geom_with_index);

