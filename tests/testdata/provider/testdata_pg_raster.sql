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


BEGIN;
CREATE TABLE "aspect_clipped_gpu_mini" ("rid" serial PRIMARY KEY,"rast" raster,"filename" text);
CREATE TABLE "o_2_aspect_clipped_gpu_mini" ("rid" serial PRIMARY KEY,"rast" raster,"filename" text);
CREATE TABLE "o_4_aspect_clipped_gpu_mini" ("rid" serial PRIMARY KEY,"rast" raster,"filename" text);
INSERT INTO "aspect_clipped_gpu_mini" ("rast","filename") VALUES ('0100000100000000000000394000000000000039C000000000D9204F41000000008F8B42410000000000000000000000000000000029230000060005004A003C1CC66A610843880B0E431CC2194306342543B7633C43861858436E0A1143BBAD194359612743A12B334317BE4343DECE59432B621B43F0E42843132B3843AC824043E6CF48436E465A435C4D2D430FA63D43F87A4843B5494A4349454E4374F35B43906E41433AB54C43B056504358575243B1EC574322615F43'::raster,'aspect_clipped_gpu_mini.tif');
INSERT INTO "o_2_aspect_clipped_gpu_mini" ("rast","filename") VALUES ('0100000100000000000000494000000000000049C000000000D9204F41000000008F8B42410000000000000000000000000000000029230000030003004A003C1CC6880B0E430634254386185843F0E42843AC8240436E465A433AB54C435857524322615F43'::raster,'aspect_clipped_gpu_mini.tif');
INSERT INTO "o_4_aspect_clipped_gpu_mini" ("rast","filename") VALUES ('0100000100000000000000594000000000000059C000000000D9204F41000000008F8B42410000000000000000000000000000000029230000020001004A003C1CC6F0E42843E6CF4843'::raster,'aspect_clipped_gpu_mini.tif');
CREATE INDEX ON "aspect_clipped_gpu_mini" USING gist (st_convexhull("rast"));
ANALYZE "aspect_clipped_gpu_mini";
CREATE INDEX ON "o_2_aspect_clipped_gpu_mini" USING gist (st_convexhull("rast"));
ANALYZE "o_2_aspect_clipped_gpu_mini";
CREATE INDEX ON "o_4_aspect_clipped_gpu_mini" USING gist (st_convexhull("rast"));
ANALYZE "o_4_aspect_clipped_gpu_mini";
SELECT AddRasterConstraints('','aspect_clipped_gpu_mini','rast',TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE);
SELECT AddRasterConstraints('','o_2_aspect_clipped_gpu_mini','rast',TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE);
SELECT AddRasterConstraints('','o_4_aspect_clipped_gpu_mini','rast',TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE);
SELECT AddOverviewConstraints('','o_2_aspect_clipped_gpu_mini','rast','','aspect_clipped_gpu_mini','rast',2);
SELECT AddOverviewConstraints('','o_4_aspect_clipped_gpu_mini','rast','','aspect_clipped_gpu_mini','rast',4);
END;
