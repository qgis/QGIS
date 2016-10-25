-- spatialite gdal_220.autotest.ogr_spatialite_views_writable.sqlite < gdal_220.autotest.ogr_spatialite_views_writable.sql
SELECT DateTime('now'),'ggdal_220.autotest.ogr_spatialite_views_writable.sql [begin] - Testing SpatialViews INSERT,UPDATE and DELETE Functionality.';
SELECT DateTime('now'),'CREATINGing positions TABLE';
CREATE TABLE positions
(
 id_admin INTEGER  PRIMARY KEY AUTOINCREMENT,
 name TEXT DEFAULT '',
 notes TEXT DEFAULT '',
 valid_since DATE DEFAULT '0001-01-01',
 valid_until DATE DEFAULT '3000-01-01'
);
-------
SELECT DateTime('now'),'CREATINGing wsg84_center GEOMETRY';
SELECT AddGeometryColumn('positions','wsg84_center',4326,'POINT','XY');
SELECT CreateSpatialIndex('positions','wsg84_center');
---
INSERT INTO positions (name,notes,valid_since,valid_until, wsg84_center) VALUES ('Brandenburger Tor','Pariser Platz','1791-08-06','3000-01-01',GeomFromEWKT('SRID=4326;POINT(13.37770458660236 52.51627178856762)'));
-------
SELECT DateTime('now'),'CREATINGing positions_1925 VIEW with INSERT TRIGGER only';
CREATE VIEW IF NOT EXISTS positions_1925 AS
 SELECT
  id_admin, name, notes, valid_since, valid_until, wsg84_center
 FROM positions
 WHERE
  ('1925-01-01' BETWEEN valid_since AND valid_until);
INSERT INTO views_geometry_columns
 (view_name,view_geometry,view_rowid,f_table_name,f_geometry_column,read_only)
 VALUES ('positions_1925','wsg84_center','id_admin','positions','wsg84_center',0);
 ---
CREATE TRIGGER vw_ins_positions_1925
 INSTEAD OF INSERT ON positions_1925
BEGIN
 INSERT OR REPLACE INTO positions
  (id_admin,name,notes,valid_since,valid_until,wsg84_center)
 VALUES(NEW.id_admin,NEW.name,NEW.notes,
  NEW.valid_since,NEW.valid_until,NEW.wsg84_center);
END;
-------
SELECT DateTime('now'),'INSERTing Siegessäule - Königs Platz into positions_1925 VIEW  - TRIGGER INSERT TEST';
INSERT INTO positions_1925 (name,notes,valid_since,valid_until, wsg84_center) VALUES ('Siegessäule','Königs Platz','1873-09-02','1938-12-31',GeomFromEWKT('SRID=4326;POINT(13.37213341515509 52.51860416707504)'));
---
SELECT DateTime('now'),'INSERTing Ampelanlage - Potsdamer Platz, Verkehrsinsel into positions_1925 VIEW  - TRIGGER INSERT TEST';
INSERT INTO positions_1925 (name,notes,valid_since,valid_until, wsg84_center) VALUES ('Ampelanlage','Potsdamer Platz, Verkehrsinsel','1924-10-24','1937-10-01',GeomFromEWKT('SRID=4326;POINT(13.37650620076562 52.5095227995198)'));
SELECT UpdateLayerStatistics('positions_1925','wsg84_center');
-------
SELECT DateTime('now'),'CREATINGing positions_1955 VIEW with INSERT and UPDATE TRIGGERs only';
CREATE VIEW IF NOT EXISTS positions_1955 AS
 SELECT
  id_admin, name, notes, valid_since, valid_until, wsg84_center
 FROM positions
 WHERE
  ('1955-08-07' BETWEEN valid_since AND valid_until);
INSERT INTO views_geometry_columns
 (view_name,view_geometry,view_rowid,f_table_name,f_geometry_column,read_only)
 VALUES ('positions_1955','wsg84_center','id_admin','positions','wsg84_center',0);
-------
CREATE TRIGGER vw_ins_positions_1955
 INSTEAD OF INSERT ON positions_1955
BEGIN
 INSERT OR REPLACE INTO positions
  (id_admin,name,notes,valid_since,valid_until,wsg84_center)
 VALUES(NEW.id_admin,NEW.name,NEW.notes,
  NEW.valid_since,NEW.valid_until,NEW.wsg84_center);
END;
---
CREATE TRIGGER vw_upd_positions_1955
 INSTEAD OF UPDATE OF 
  name,notes,valid_since,valid_until,wsg84_center
 ON positions_1955
BEGIN
 UPDATE positions
 SET 
  name = NEW.name, 
  notes = NEW.notes,
  valid_since = NEW.valid_since, 
  valid_until = NEW.valid_until, 
  wsg84_center = NEW.wsg84_center
  -- the primary key known to the view ḿust be used
 WHERE id_admin = OLD.id_admin;
END;
-------
SELECT DateTime('now'),'CREATINGing positions_1999 VIEW with INSERT, UPDATE and DELETE TRIGGERs';
CREATE VIEW IF NOT EXISTS positions_1999 AS
 SELECT
  id_admin, name, notes, valid_since, valid_until, wsg84_center
 FROM positions
 WHERE
  ('1999-01-01' BETWEEN valid_since AND valid_until);
INSERT INTO views_geometry_columns
 (view_name,view_geometry,view_rowid,f_table_name,f_geometry_column,read_only)
 VALUES ('positions_1999','wsg84_center','id_admin','positions','wsg84_center',0);
-------
CREATE TRIGGER vw_ins_positions_1999
 INSTEAD OF INSERT ON positions_1999
BEGIN
 INSERT OR REPLACE INTO positions
  (id_admin,name,notes,valid_since,valid_until,wsg84_center)
 VALUES(NEW.id_admin,NEW.name,NEW.notes,
  NEW.valid_since,NEW.valid_until,NEW.wsg84_center);
END;
-------
CREATE TRIGGER vw_upd_positions_1999
 INSTEAD OF UPDATE OF 
  name,notes,valid_since,valid_until,wsg84_center
 ON positions_1999
BEGIN
 UPDATE positions
 SET 
  name = NEW.name, 
  notes = NEW.notes,
  valid_since = NEW.valid_since, 
  valid_until = NEW.valid_until, 
  wsg84_center = NEW.wsg84_center
  -- the primary key known to the view ḿust be used
 WHERE id_admin = OLD.id_admin;
END;
-------
CREATE TRIGGER vw_del_positions_1999
 INSTEAD OF DELETE ON 
 positions_1999
BEGIN
 DELETE FROM positions WHERE id_admin = OLD.id_admin;
END;
-------
SELECT DateTime('now'),'INSERTing Siegessäule - Große Stern into positions_1999 VIEW  - TRIGGER INSERT TEST';
INSERT INTO positions_1999 (name,notes,valid_since,valid_until, wsg84_center) VALUES ('Siegessäule','Große Stern','1939-01-01','3000-01-01',GeomFromEWKT('SRID=4326;POINT(13.35009574853021 52.51451079619255)'));
-------
SELECT DateTime('now'),'INSERTing Ampelanlage - Potsdamer Platz into positions_1999 VIEW  - TRIGGER INSERT TEST';
INSERT INTO positions_1999 (name,notes,valid_since,valid_until, wsg84_center) VALUES ('Ampelanlage','Potsdamer Platz','1998-10-02','3000-01-01',GeomFromEWKT('SRID=4326;POINT(13.37625537334001 52.50926345498337)'));
-------
SELECT DateTime('now'),'UpdateLayerStatistics: wsg84_center';
SELECT UpdateLayerStatistics('positions_1955','wsg84_center');
SELECT UpdateLayerStatistics('positions_1999','wsg84_center');
SELECT UpdateLayerStatistics('positions','wsg84_center');
-------
SELECT DateTime('now'),'Statistics: of Table,Views: should be [5,3,2,3 - with 5 unique name-notes and positions]';
SELECT DateTime('now'),'count_positions('||(SELECT count(*) FROM positions)||') ; count_positions_1925('||(SELECT count(*) FROM positions_1925)||') ; count_positions_1955('||(SELECT count(*) FROM positions_1955)||') ; count_positions_1999('||(SELECT count(*) FROM positions_1999)||')' AS result_count;
SELECT DateTime('now'),'DISTINCT name and notes('||count(DISTINCT name||notes)||') positions('||count(DISTINCT wsg84_center)||') from positions'  AS distinct_results  FROM positions;
SELECT DateTime('now'),'SELECT: of positions_1925';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1925;
SELECT DateTime('now'),'SELECT: of positions_1955 (no Ampelanlage - Potsdamer Platz)';
SELECT '1955',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1955;
SELECT DateTime('now'),'SELECT: of positions_1999';
SELECT '1999',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1999;
-------
SELECT DateTime('now'),'Replacing Siegessäule position of 1939 with Original Position of 1873 - ''positions_1955'' TRIGGER UPDATE TEST';
BEGIN;
--
UPDATE positions_1955 
 SET wsg84_center=
 (
  SELECT wsg84_center FROM positions_1955 WHERE (name = 'Siegessäule')
 ),
 notes=
 (
  SELECT notes FROM positions_1955 WHERE (name = 'Siegessäule')
 )
WHERE (name = 'Siegessäule');
--
SELECT DateTime('now'),'Statistics: of Table,Views after UPDATE:  should be [5,3,2,3 - with 2 unique name-notes and positions]';
SELECT DateTime('now'),'count_positions('||(SELECT count(*) FROM positions)||') ; count_positions_1925('||(SELECT count(*) FROM positions_1925)||') ; count_positions_1955('||(SELECT count(*) FROM positions_1955)||') ; count_positions_1999('||(SELECT count(*) FROM positions_1999)||')' AS result_count;
SELECT DateTime('now'),'DISTINCT name and notes('||count(DISTINCT name||notes)||') positions('||count(DISTINCT wsg84_center)||') from positions'  AS distinct_results  FROM positions;
SELECT DateTime('now'),'SELECT: of positions_1925';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1925;
SELECT DateTime('now'),'SELECT: of positions_1955 (no Ampelanlage - Potsdamer Platz)';
SELECT '1955',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1955;
SELECT DateTime('now'),'SELECT: of positions_1999';
SELECT '1999',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1999;
--
SELECT DateTime('now'),'DELETE Siegessäule from positions_1999 - ''positions_1999'' TRIGGER DELETE TEST';
DELETE FROM positions_1999  WHERE (name = 'Siegessäule');
--
SELECT DateTime('now'),'Statistics: of Table,Views after DELETE: should be [4,3,2,1 - with 4 unique name-notes and positions]';
SELECT DateTime('now'),'count_positions('||(SELECT count(*) FROM positions)||') ; count_positions_1925('||(SELECT count(*) FROM positions_1925)||') ; count_positions_1955('||(SELECT count(*) FROM positions_1955)||') ; count_positions_1999('||(SELECT count(*) FROM positions_1999)||')' AS result_count;
SELECT DateTime('now'),'DISTINCT name and notes('||count(DISTINCT name||notes)||') positions('||count(DISTINCT wsg84_center)||') from positions'  AS distinct_results  FROM positions;
SELECT DateTime('now'),'SELECT: of positions_1925';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1925;
SELECT DateTime('now'),'SELECT: of positions_1955 (no Ampelanlage - Potsdamer Platz)';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1955;
SELECT DateTime('now'),'SELECT: of positions_1999';
SELECT '1999',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1999;
--
SELECT DateTime('now'),'Rollback of changes';
ROLLBACK;
--
SELECT DateTime('now'),'Statistics: of Table,Views after ROLLBACK: should be [5,3,2,3 - with 5 unique name-notes and positions]';
SELECT DateTime('now'),'count_positions('||(SELECT count(*) FROM positions)||') ; count_positions_1925('||(SELECT count(*) FROM positions_1925)||') ; count_positions_1955('||(SELECT count(*) FROM positions_1955)||') ; count_positions_1999('||(SELECT count(*) FROM positions_1999)||')' AS result_count;
SELECT DateTime('now'),'DISTINCT name and notes('||count(DISTINCT name||notes)||') positions('||count(DISTINCT wsg84_center)||') from positions'  AS distinct_results  FROM positions;
SELECT DateTime('now'),'SELECT: of positions_1925';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1925;
SELECT DateTime('now'),'SELECT: of positions_1955 (no Ampelanlage - Potsdamer Platz)';
SELECT '1925',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1955;
SELECT DateTime('now'),'SELECT: of positions_1999';
SELECT '1999',name,notes,valid_since,valid_until, AsEWKT(wsg84_center)  FROM positions_1999;
--
SELECT DateTime('now'),'gdal_220.autotest.ogr_spatialite_views_writable.sql [finished] [Habe fertig!]';
--
