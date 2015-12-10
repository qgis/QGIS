CREATE SCHEMA qgis_test

CREATE TABLE qgis_test.[someData] (
    pk integer PRIMARY KEY,
    cnt integer,
    name nvarchar(max) DEFAULT 'qgis',
    name2 nvarchar(max) DEFAULT 'qgis',
    geom geometry
)

INSERT INTO qgis_test.[someData] (pk, cnt, name, name2, geom) VALUES
(5, -200, NULL, 'NuLl', geometry::STGeomFromText( 'Point(-71.123 78.23)', 4326 )),
(3,  300, 'Pear', 'PEaR', NULL),
(1,  100, 'Orange', 'oranGe', geometry::STGeomFromText( 'Point(-70.332 66.33)', 4326 )),
(2,  200, 'Apple', 'Apple', geometry::STGeomFromText( 'Point(-68.2 70.8)', 4326 )),
(4,  400, 'Honey', 'Honey', geometry::STGeomFromText( 'Point(-65.32 78.3)', 4326 ))
;
