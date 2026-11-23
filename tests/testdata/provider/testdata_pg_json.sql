
DROP TABLE IF EXISTS qgis_test.json;

CREATE TABLE qgis_test.json
(
  pk SERIAL NOT NULL PRIMARY KEY,
  jvalue json,
  jbvalue jsonb,
  textfield text
);

INSERT INTO qgis_test.json(jvalue, jbvalue, textfield)
  VALUES
    ('[1,2,3]', '[4,5,6]', '[7,8,9]'),
    ('{"a":1,"b":2}', '{"c":4,"d":5}', '{"e":6,"f":7}');
