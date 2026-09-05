--
-- in-db float32 raster used to test QgsRasterLayer reload:
-- Companion update: raster_3035_reload_update.sql
--

DROP TABLE IF EXISTS "public"."raster_3035_reload";

CREATE TABLE "public"."raster_3035_reload" (
    "pk" SERIAL PRIMARY KEY,
    "rast" raster
);

INSERT INTO "public"."raster_3035_reload" ("rast")
VALUES (
    ST_AddBand(
        ST_MakeEmptyRaster(2, 2, 4080050, 2430750, 25, -25, 0, 0, 3035),
        '32BF'::text, 100, -9999
    )
);

CREATE INDEX ON "public"."raster_3035_reload" USING gist (st_convexhull("rast"));
ANALYZE "public"."raster_3035_reload";
