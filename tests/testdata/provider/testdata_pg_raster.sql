DO $$
BEGIN
  IF EXISTS ( SELECT * FROM pg_catalog.pg_available_extensions
              WHERE name = 'postgis_raster' )
  THEN
    RAISE NOTICE 'Loading postgis_raster';
    CREATE EXTENSION IF NOT EXISTS postgis_raster;
  END IF;
END;
$$;

-- Table: qgis_test.Raster1

CREATE TABLE qgis_test."Raster1"
(
  pk serial NOT NULL,
  name character varying(255),
  "Rast" raster
);

INSERT INTO qgis_test."Raster1" (name, "Rast") SELECT
  'simple one',
  ST_AddBand(
    ST_MakeEmptyRaster(16, 32, 7, -5, 0.2, -0.7, 0, 0, 0),
    1, '8BUI', 0.0, NULL
  );
