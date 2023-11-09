
DROP TABLE IF EXISTS qgis_test.string_array;

CREATE TABLE qgis_test.string_array
(
  pk SERIAL NOT NULL PRIMARY KEY,
  value text[]
);

INSERT INTO qgis_test.string_array(value)
  VALUES
    ('{a,b,c}');


DROP TABLE IF EXISTS qgis_test.int_array;

CREATE TABLE qgis_test.int_array
(
  pk SERIAL NOT NULL PRIMARY KEY,
  value int4[]
);

INSERT INTO qgis_test.int_array(value)
  VALUES
    ('{1,2,-5}');

DROP TABLE IF EXISTS qgis_test.double_array;

CREATE TABLE qgis_test.double_array
(
  pk SERIAL NOT NULL PRIMARY KEY,
  value float8[]
);

INSERT INTO qgis_test.double_array(value)
  VALUES
    ('{1.1,2,-5.12345}');
