DROP TABLE qgis_test.someData;

CREATE TABLE qgis_test.someData (
    pk integer NOT NULL PRIMARY KEY,
    cnt integer,
    name varchar(32) DEFAULT 'qgis',
    name2 varchar(32) DEFAULT 'qgis',
	num_char char(1),
    geom db2gse.st_point
)
;
INSERT INTO qgis_test.someData (pk, cnt, name, name2, num_char, geom) VALUES
(5, -200, NULL, 'NuLl', '5', db2gse.st_point( 'Point(-71.123 78.23)', 4326 )),
(3,  300, 'Pear', 'PEaR', '3', NULL),
(1,  100, 'Orange', 'oranGe', '1', db2gse.st_point( 'Point(-70.332 66.33)', 4326 )),
(2,  200, 'Apple', 'Apple', '2', db2gse.st_point( 'Point(-68.2 70.8)', 4326 )),
(4,  400, 'Honey', 'Honey', '4', db2gse.st_point( 'Point(-65.32 78.3)', 4326 ))
;
