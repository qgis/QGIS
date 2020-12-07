


CREATE DOMAIN public.colordomain
  AS text
  COLLATE pg_catalog."default"
  CONSTRAINT domainconstraint CHECK (VALUE = ANY (ARRAY['red'::text, 'green'::text, 'blue'::text]));

CREATE DOMAIN qgis_test.colordomain
  AS text
  COLLATE pg_catalog."default"
  CONSTRAINT domainconstraint CHECK (VALUE = ANY (ARRAY['yellow'::text, 'cyan'::text, 'magenta'::text]));

CREATE DOMAIN qgis_test.flowerdomain
  AS text
  COLLATE pg_catalog."default"
  CONSTRAINT domainconstraint CHECK (VALUE = ANY (ARRAY['Alchemilla'::text, 'Alstroemeria'::text, 'Alyssum'::text]));

CREATE TABLE qgis_test.colors
(
id SERIAL NOT NULL,
color_public colordomain,
color_qgis qgis_test.colordomain,
flower qgis_test.flowerdomain
)
