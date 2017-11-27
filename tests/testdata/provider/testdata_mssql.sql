CREATE SCHEMA qgis_test

CREATE TABLE qgis_test.[someData] (
    pk integer PRIMARY KEY,
    cnt integer,
    name nvarchar(max) DEFAULT 'qgis',
    name2 nvarchar(max) DEFAULT 'qgis',
    num_char nvarchar(max),
    geom geometry
);

CREATE TABLE qgis_test.[some_poly_data] (
    pk integer PRIMARY KEY,
    geom geometry
);

CREATE TABLE qgis_test.[date_times] (
       id integer PRIMARY KEY,
       date_field date,
       time_field time,
       datetime_field datetime
);

INSERT INTO qgis_test.[someData] (pk, cnt, name, name2, num_char, geom) VALUES
(5, -200, NULL, 'NuLl', '5', geometry::STGeomFromText( 'Point(-71.123 78.23)', 4326 )),
(3,  300, 'Pear', 'PEaR', '3', NULL),
(1,  100, 'Orange', 'oranGe', '1', geometry::STGeomFromText( 'Point(-70.332 66.33)', 4326 )),
(2,  200, 'Apple', 'Apple', '2', geometry::STGeomFromText( 'Point(-68.2 70.8)', 4326 )),
(4,  400, 'Honey', 'Honey', '4', geometry::STGeomFromText( 'Point(-65.32 78.3)', 4326 ))
;

INSERT INTO qgis_test.[some_poly_data] (pk, geom) VALUES
(1, geometry::STGeomFromText('Polygon ((-69.0 81.4, -69.0 80.2, -73.7 80.2, -73.7 76.3, -74.9 76.3, -74.9 81.4, -69.0 81.4))', 4326 )),
(2, geometry::STGeomFromText('Polygon ((-67.6 81.2, -66.3 81.2, -66.3 76.9, -67.6 76.9, -67.6 81.2))', 4326 )),
(3, geometry::STGeomFromText('Polygon ((-68.4 75.8, -67.5 72.6, -68.6 73.7, -70.2 72.9, -68.4 75.8))', 4326 )),
(4, NULL)
;


INSERT INTO qgis_test.[date_times] (id, date_field, time_field, datetime_field ) VALUES
 (1, '2004-03-04', '13:41:52', '2004-03-04 13:41:52' );
