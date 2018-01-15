
CREATE EXTENSION IF NOT EXISTS hstore;

DROP TABLE IF EXISTS qgis_test.dict;

CREATE TABLE qgis_test.dict
(
  pk SERIAL NOT NULL PRIMARY KEY,
  value hstore
);

INSERT INTO qgis_test.dict(value)
  VALUES
    ('a=>b,1=>2');
