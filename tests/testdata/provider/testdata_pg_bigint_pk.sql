DROP TABLE IF EXISTS qgis_test.bigint_pk;

CREATE TABLE qgis_test.bigint_pk (
  pk bigserial NOT NULL PRIMARY KEY,
  value varchar(16),
  geom geometry(Point, 4326)
);

INSERT INTO qgis_test.bigint_pk (value, geom)
  VALUES
    ('first value', ST_SetSRID(ST_MakePoint(-60.1, 1.0), 4326)),
    ('second value', ST_SetSRID(ST_MakePoint(-61.1, -1.0), 4326));

INSERT INTO qgis_test.bigint_pk (pk, value, geom)
  VALUES
    (0, 'zero value', ST_SetSRID(ST_MakePoint(-49.1, 1.0), 4326)),
    (-1, 'negative value', ST_SetSRID(ST_MakePoint(-45.1, 1.0), 4326));

DROP TABLE IF EXISTS qgis_test.bigint_non_first_pk;

CREATE TABLE qgis_test.bigint_non_first_pk (
  value varchar(16),
  pk bigserial NOT NULL PRIMARY KEY,
  geom geometry(Point, 4326)
);

INSERT INTO qgis_test.bigint_non_first_pk (value, geom)
  VALUES
    ('first value', ST_SetSRID(ST_MakePoint(-60.1, 1.0), 4326)),
    ('second value', ST_SetSRID(ST_MakePoint(-61.1, -1.0), 4326));

INSERT INTO qgis_test.bigint_non_first_pk (pk, value, geom)
  VALUES
    (0, 'zero value', ST_SetSRID(ST_MakePoint(-49.1, 1.0), 4326)),
    (-1, 'negative value', ST_SetSRID(ST_MakePoint(-45.1, 1.0), 4326));

DROP TABLE IF EXISTS qgis_test.bigint_composite_pk;

CREATE TABLE qgis_test.bigint_composite_pk (
  intid BIGINT NOT NULL,
  charid varchar(8) NOT NULL,
  value varchar(16),
  geom geometry(Point, 4326),
  PRIMARY KEY(intid, charid)
);

INSERT INTO qgis_test.bigint_composite_pk (intid, charid, value, geom)
  VALUES
    (1, '1', 'first value', ST_SetSRID(ST_MakePoint(-60.1, 1.0), 4326)),
    (2, '2', 'second value', ST_SetSRID(ST_MakePoint(-61.1, -0.5), 4326)),
    (0, '0', 'zero value', ST_SetSRID(ST_MakePoint(-58.1, -0.5), 4326)),
    (-1, '-1', 'negative value', ST_SetSRID(ST_MakePoint(-58.1, -0.5), 4326));

/* -- PostgreSQL 12 or later

DROP TABLE IF EXISTS qgis_test.bigint_partitioned;

CREATE TABLE qgis_test.bigint_partitioned (
  pk BIGSERIAL NOT NULL,
  value varchar(8),
  geom geometry(Point, 4326)
) PARTITION BY RANGE(pk);

CREATE TABLE qgis_test.bigint_partitioned_positive PARTITION OF qgis_test.bigint_partitioned
  FOR VALUES FROM 1 TO 1000;
CREATE TABLE qgis_test.bigint_partitioned_nonpositive PARTITION OF qgis_test.bigint_partitioned
  FOR VALUES FROM -1 TO 0;

ALTER TABLE qgis_test.bigint_partitioned ADD PRIMARY KEY(pk);
*/
