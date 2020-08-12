--
-- in-db float32 untiled raster with multiple rows
-- also used to test temporal capabilities
--

DROP TABLE IF EXISTS "public"."raster_3035_untiled_multiple_rows";

CREATE TABLE "public"."raster_3035_untiled_multiple_rows" (
    "pk" SERIAL PRIMARY KEY,
    "rast" raster,
    "data" timestamp,
    "data_text" varchar
);

INSERT INTO "public"."raster_3035_untiled_multiple_rows" ("rast", "pk", "data", "data_text")
VALUES ('0100000100000000000000394000000000000039C000000000D9204F41000000008F8B424100000000000000000000000000000000DB0B0000020002004A003C1CC66A610843880B0E436E0A1143BBAD1943'::raster, 1, '2020-04-01', '2020-04-01');
INSERT INTO "public"."raster_3035_untiled_multiple_rows" ("rast", "pk", "data", "data_text")
VALUES ('0100000100000000000000394000000000000039C000000000D9204F41000000008F8B424100000000000000000000000000000000DB0B0000020002004A003C1CC66A610843880B0E436E0A2143BBAD2943'::raster, 2, '2020-04-05', '2020-04-05');
-- offset row
INSERT INTO "public"."raster_3035_untiled_multiple_rows" ("rast", "pk", "data", "data_text")
VALUES ('0100000100000000000000394000000000000039C000000000D9204F41000000008F8B424100000000000000000000000000000000DB0B0000020002004A003C1CC66A610843880B0E436E0A2143BBAD0942'::raster, 3, '2020-04-06', '2020-04-06');

CREATE INDEX ON "public"."raster_3035_untiled_multiple_rows" USING gist (st_convexhull("rast"));
ANALYZE "public"."raster_3035_untiled_multiple_rows";
SELECT AddRasterConstraints('public','raster_3035_untiled_multiple_rows','rast',TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE);
