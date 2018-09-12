
DROP TABLE IF EXISTS qgis_test.json;

CREATE TABLE qgis_test.json
(
  pk SERIAL NOT NULL PRIMARY KEY,
  jvalue json,
  jbvalue jsonb
);

INSERT INTO qgis_test.json(jvalue, jbvalue)
  VALUES
    ('[1,2,3]', '[4,5,6]'),
    ('{"a":1,"b":2}', '{"c":4,"d":5}');
