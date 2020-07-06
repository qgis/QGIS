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
    dt timestamp without time zone,
    "date" date,
    "time" time without time zone,
    geom public.geometry(Point,4326),
    key1 integer,
    key2 integer,
    PRIMARY KEY(pk)
);

INSERT INTO qgis_test.provider_bigint_single_pk  ( key1, key2, pk, cnt, name, name2, num_char, dt, "date", "time", geom) VALUES
(1, 1, 5, -200, NULL, 'NuLl', '5', TIMESTAMP '2020-05-04 12:13:14', '2020-05-02', '12:13:01', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL, NULL, NULL, NULL),
(2, 1, 1,  100, 'Orange', 'oranGe', '1', TIMESTAMP '2020-05-03 12:13:14', '2020-05-03', '12:13:14', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(2, 2, 2,  200, 'Apple', 'Apple', '2', TIMESTAMP '2020-05-04 12:14:14', '2020-05-04', '12:14:14', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(2, 3, 4,  400, 'Honey', 'Honey', '4', TIMESTAMP '2021-05-04 13:13:14', '2021-05-04', '13:13:14', '0101000020E610000014AE47E17A5450C03333333333935340')
;

DROP TABLE IF EXISTS qgis_test.provider_bigint_nonfirst_pk;

CREATE TABLE qgis_test.provider_bigint_nonfirst_pk (
    zeroth_field integer,
    primkey bigint NOT NULL,
    cnt integer,
    name text DEFAULT 'qgis',
    name2 text DEFAULT 'qgis',
    num_char text,
    dt timestamp without time zone,
    "date" date,
    "time" time without time zone,
    geom public.geometry(Point,4326),
    key1 integer,
    key2 integer,
    PRIMARY KEY(primkey)
);

INSERT INTO qgis_test.provider_bigint_nonfirst_pk  (zeroth_field, key1, key2, primkey, cnt, name, name2, num_char, dt, "date", "time", geom) VALUES
(-3, 1, 1, 5, -200, NULL, 'NuLl', '5', TIMESTAMP '2020-05-04 12:13:14', '2020-05-02', '12:13:01', '0101000020E61000001D5A643BDFC751C01F85EB51B88E5340'),
(-2, 1, 2, 3,  300, 'Pear', 'PEaR', '3', NULL, NULL, NULL, NULL),
(-1, 2, 1, 1,  100, 'Orange', 'oranGe', '1', TIMESTAMP '2020-05-03 12:13:14', '2020-05-03', '12:13:14', '0101000020E61000006891ED7C3F9551C085EB51B81E955040'),
(0, 2, 2, 2,  200, 'Apple', 'Apple', '2', TIMESTAMP '2020-05-04 12:14:14', '2020-05-04', '12:14:14', '0101000020E6100000CDCCCCCCCC0C51C03333333333B35140'),
(1, 2, 3, 4,  400, 'Honey', 'Honey', '4', TIMESTAMP '2021-05-04 13:13:14', '2021-05-04', '13:13:14', '0101000020E610000014AE47E17A5450C03333333333935340')
;

CREATE TABLE qgis_test.tb_test_compound_pk
(
    pk1 INTEGER,
    pk2 BIGINT,
    value VARCHAR(16),
    geom geometry(Point, 4326),
    PRIMARY KEY (pk1, pk2)
);

INSERT INTO qgis_test.tb_test_compound_pk (pk1, pk2, value, geom) VALUES
    (1, 1, 'test 1', ST_SetSRID(ST_Point(-47.930, -15.818), 4326)),
    (1, 2, 'test 2', ST_SetSRID(ST_Point(-47.887, -15.864), 4326)),
    (2, 1, 'test 3', ST_SetSRID(ST_Point(-47.902, -15.763), 4326)),
    (2, 2, 'test 4', ST_SetSRID(ST_Point(-47.952, -15.781), 4326));

CREATE TABLE qgis_test.tb_test_composite_float_pk
(
    pk1 INTEGER,
    pk2 BIGINT,
    pk3 REAL,
    value VARCHAR(16),
    geom geometry(Point, 4326),
    PRIMARY KEY (pk1, pk2, pk3)
);

INSERT INTO qgis_test.tb_test_composite_float_pk (pk1, pk2, pk3, value, geom) VALUES
    (1, 1, 1.0,         'test 1', ST_SetSRID(ST_Point(-47.930, -15.818), 4326)),
    (1, 2, 3.141592741, 'test 2', ST_SetSRID(ST_Point(-47.887, -15.864), 4326)),
    (2, 2, 2.718281828, 'test 3', ST_SetSRID(ST_Point(-47.902, -15.763), 4326)),
    (2, 2, 1.0,         'test 4', ST_SetSRID(ST_Point(-47.952, -15.781), 4326));

CREATE TABLE qgis_test.tb_test_float_pk
(
    pk REAL PRIMARY KEY,
    value VARCHAR(16),
    geom geometry(Point, 4326)
);
-- those values (pi, Euler's, and a third) will be truncated/rounded to fit
-- PostgreSQL's internal type size. REAL is IEEE-754 4 bytes (32 bit).
INSERT INTO qgis_test.tb_test_float_pk (pk, value, geom) VALUES
    (3.141592653589793238462643383279502884197169399375105820974944592307816406286, 'first teste', ST_SetSRID(ST_Point(-47.887, -15.864), 4326)),
    (2.718281828459045235360287471352662497757247093699959574966967627724076630353, 'second test', ST_SetSRID(ST_Point(-47.902, -15.763), 4326)),
    (1.333333333333333333333333333333333333333333333333333333333333333333333333333, 'third teste', ST_SetSRID(ST_Point(-47.751, -15.644), 4326));

CREATE TABLE qgis_test.tb_test_double_pk
(
    pk DOUBLE PRECISION PRIMARY KEY,
    value VARCHAR(16),
    geom geometry(Point, 4326)
);
-- those values (pi, Euler's, and a third) will be truncated/rounded to fit
-- PostgreSQL's internal type size. DOUBLE PRECISION is IEEE-754 8 bytes (64 bit).
INSERT INTO qgis_test.tb_test_double_pk (pk, value, geom) VALUES
    (3.141592653589793238462643383279502884197169399375105820974944592307816406286, 'first teste', ST_SetSRID(ST_Point(-47.887, -15.864), 4326)),
    (2.718281828459045235360287471352662497757247093699959574966967627724076630353, 'second test', ST_SetSRID(ST_Point(-47.902, -15.763), 4326)),
    (1.333333333333333333333333333333333333333333333333333333333333333333333333333, 'third teste', ST_SetSRID(ST_Point(-47.751, -15.644), 4326));

