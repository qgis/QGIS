
DROP TABLE IF EXISTS qgis_test.testgeog CASCADE;

CREATE TABLE qgis_test.testgeog
(
  pk SERIAL NOT NULL PRIMARY KEY,
  geog GEOGRAPHY
);

INSERT INTO qgis_test.testgeog(geog) VALUES ('POINT(40 0)'::geography);
INSERT INTO qgis_test.testgeog(geog) VALUES ('POINT(40 60)'::geography);
INSERT INTO qgis_test.testgeog(geog) VALUES ('POINT(40 -60)'::geography);
INSERT INTO qgis_test.testgeog(geog) VALUES ('POINT(180 45)'::geography);

CREATE INDEX testgeog_geog_idx ON qgis_test.testgeog USING GIST ( geog );
