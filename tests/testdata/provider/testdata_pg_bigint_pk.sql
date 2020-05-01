DROP TABLE IF EXISTS qgis_test.bigint_pk;

CREATE TABLE qgis_test.bigint_pk (
  pk bigserial NOT NULL PRIMARY KEY,
  value varchar(16),
  bigint_attribute bigint,
  bigint_attribute_def bigint DEFAULT 42,
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

DROP TABLE IF EXISTS qgis_test.provider_bigint_single_pk;

CREATE TABLE qgis_test.provider_bigint_single_pk (
    pk bigint NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    name2 text DEFAULT 'qgis',
    num_char text,
    geom public.geometry(Point,4326),
    key1 integer,
    key2 integer,
    PRIMARY KEY(pk)
);

INSERT INTO qgis_test.provider_bigint_single_pk  ( key1, key2, pk, cnt, name, name2, num_char, geom) VALUES
(1, 1, 5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL),
(2, 1, 1,  100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(2, 2, 2,  200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(2, 3, 4,  400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')
;

DROP TABLE IF EXISTS qgis_test.provider_bigint_nonfirst_pk;

CREATE TABLE qgis_test.provider_bigint_nonfirst_pk (
    zeroth_field integer,
    primkey bigint NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    name2 text DEFAULT 'qgis',
    num_char text,
    geom public.geometry(Point,4326),
    key1 integer,
    key2 integer,
    PRIMARY KEY(primkey)
);

INSERT INTO qgis_test.provider_bigint_nonfirst_pk  (zeroth_field, key1, key2, primkey, cnt, name, name2, num_char, geom) VALUES
(-3, 1, 1, 5, -200, NULL, 'NuLl', '5', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(-2, 1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL),
(-1, 2, 1, 1,  100, 'Orange', 'oranGe', '1', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(0, 2, 2, 2,  200, 'Apple', 'Apple', '2', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(1, 2, 3, 4,  400, 'Honey', 'Honey', '4', '0101000020E610000014AE47E17A5450C03333333333935340')
;

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
