-- Postprocessing SQL commands for the srs.db
-- Run (from Linux):
-- sqlite3 srs.db <postprocess_srs.sql

-- Swedish RT-90 projections with increased accuracy
INSERT INTO tbl_srs VALUES(NULL,'RT90 7.5 gon V from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=11.306250000000 +k=1.000006000000 +x_0=1500025.141 +y_0=-667.282 +ellps=GRS80 +units=m +no_defs',93019,'EPSG',93019,0);
INSERT INTO tbl_srs VALUES(NULL,'RT90 5 gon V from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0  +lon_0=13.55626666666 +k=1.000005800000 +x_0=1500044.695 +y_0=-667.130 +ellps=GRS80 +units=m +no_defs',93020,'EPSG',93020,0);
INSERT INTO tbl_srs VALUES(NULL,'RT90 2.5 gon V from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=15.806284529444 +k=1.000005610240 +x_0=1500064.274 +y_0=-667.711 +ellps=GRS80 +units=m +no_defs',93021,'EPSG',93021,0);
INSERT INTO tbl_srs VALUES(NULL,'RT90 0 gon from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=18.056300000000 +k=1.000005400000 +x_0=1500083.521 +y_0=-668.844 +ellps=GRS80 +units=m +no_defs',93022,'EPSG',93022,0);
INSERT INTO tbl_srs VALUES(NULL,'RT90 2.5 gon O from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=20.306316666666 +k=1.000005200000 +x_0=1500102.765 +y_0=-670.706 +ellps=GRS80 +units=m +no_defs',93023,'EPSG',93023,0);
INSERT INTO tbl_srs VALUES(NULL,'RT90 5 gon O from WGS84','tmerc','GRS80',
'+proj=tmerc +lat_0=0 +lon_0=22.556333333333 +k=1.000004900000 +x_0=1500121.846 +y_0=-672.557 +ellps=GRS80 +units=m +no_defs',93024,'EPSG',93024,0);

-- S-JTSK/Krovak (Greenwich) (ticket #728)
INSERT INTO tbl_srs VALUES (NULL,'S-JTSK (Greenwich) / Krovak','krovak','bessel','+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=30.28813972222222 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +pm=greenwich +units=m +no_defs','102067','EPSG','102067','0'); 
