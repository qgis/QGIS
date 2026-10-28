--
-- Update for raster_3035_reload: add a tile, so that the layer extent, size and
-- data must all change after reload.
--

INSERT INTO "public"."raster_3035_reload" ("rast")
VALUES (
    ST_AddBand(
        ST_MakeEmptyRaster(2, 2, 4080100, 2430750, 25, -25, 0, 0, 3035),
        '32BF'::text, 200, -9999
    )
);
