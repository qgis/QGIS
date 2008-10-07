-- Postprocessing SQL commands for the srs.db
-- Run (from Linux):
-- sqlite3 srs.db <postprocess_srs.sql

REPLACE INTO tbl_srs VALUES(984,'RT90 7.5 gon V','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=11.306250000000 +k=1.000006000000 +x_0=1500025.141 +y_0=-667.282 +ellps=GRS80 +units=m +no_defs',3019,3019,0);
REPLACE INTO tbl_srs VALUES(985,'RT90 5 gon V','tmerc','GRS80',
'+proj=tmerc +lat_0=0  +lon_0=13.55626666666 +k=1.000005800000 +x_0=1500044.695 +y_0=-667.130 +ellps=GRS80 +units=m +no_defs',3020,3020,0);
REPLACE INTO tbl_srs VALUES(986,'RT90 2.5 gon V','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=15.806284529444 +k=1.000005610240 +x_0=1500064.274 +y_0=-667.711 +ellps=GRS80 +units=m +no_defs',3021,3021,0);
REPLACE INTO tbl_srs VALUES(987,'RT90 0 gon','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=18.056300000000 +k=1.000005400000 +x_0=1500083.521 +y_0=-668.844 +ellps=GRS80 +units=m +no_defs',3022,3022,0);
REPLACE INTO tbl_srs VALUES(988,'RT90 2.5 gon O','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=20.306316666666 +k=1.000005200000 +x_0=1500102.765 +y_0=-670.706 +ellps=GRS80 +units=m +no_defs',3023,3023,0);
REPLACE INTO tbl_srs VALUES(989,'RT90 5 gon O','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=22.556333333333 +k=1.000004900000 +x_0=1500121.846 +y_0=-672.557 +ellps=GRS80 +units=m +no_defs',3024,3024,0);
