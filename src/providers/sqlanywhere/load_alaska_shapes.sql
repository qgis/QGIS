/***************************************************************************
    load_alaska_shapes.sql
    A sample SQL script for loading the Alaska shapefiles from the QGIS
    sample dataset into a SQL Anywhere database.
 ***************************************************************************
    Copyright (c) 2010 iAnywhere Solutions, Inc.
    All rights reserved. All unpublished rights reserved.
 ***************************************************************************
    This sample code is provided AS IS, without warranty or liability
    of any kind.
    
    You may use, reproduce, modify and distribute this sample code
    without limitation, on the condition that you retain the foregoing
    copyright notice and disclaimer as to the original iAnywhere code.  
 ***************************************************************************/
/* $Id$ */

-- BEGIN CONFIGURATION VARIABLES
CREATE OR REPLACE VARIABLE @QGIS_SAMPLES_DIR TEXT;
CREATE OR REPLACE VARIABLE @QGIS_SAMPLES_TABLEPREFIX TEXT;
CREATE OR REPLACE VARIABLE @QGIS_SAMPLES_LIST TEXT;

SET @QGIS_SAMPLES_DIR='C:\\temp\\qgis_sample_data\\vmap0_shapefiles\\';
SET @QGIS_SAMPLES_TABLEPREFIX='QGIS_';
SET @QGIS_SAMPLES_LIST=
     'airports'
 || ',alaska'
 || ',builtups'
 || ',grassland'
 || ',lakes'
 || ',landice'
 || ',majrivers'
 || ',pipelines'
 || ',popp'
 || ',railroads'
 || ',rivers'
 || ',storagep'
 || ',swamp'
 || ',trails'
 || ',trees'
 || ',tundra'
;
-- END CONFIGURATION VARIABLES


-- SQL Anywhere 12.0.1 contains a built-in stored procedure
-- "dbo"."st_geometry_load_shapefile" to facilitate loading of shapefiles.
-- That procedure is not present in 12.0.0, so we instead define a 
-- temporary procedure here.
CREATE TEMPORARY PROCEDURE "load_shapefile"( 
  in shp_filename varchar(512),
  in srid integer,
  in table_name varchar(128) )
BEGIN
  DECLARE @EXEC_SQL TEXT;
  SET @EXEC_SQL = 'CREATE TABLE "' || table_name 
     || '"( record_number INT PRIMARY KEY, '
     || (SELECT LIST('"' || name || '" ' || domain_name_with_size,', ' order by
      column_number asc)
      FROM sa_describe_shapefile(shp_filename,srid)
      WHERE column_number > 1)
     || ' )';
  EXECUTE IMMEDIATE @EXEC_SQL;
  -- escape ' and \ to be inside a literal string
  SET shp_filename = REPLACE(REPLACE(shp_filename,'''',''''''),
    '\\','\\\\');
  SET @EXEC_SQL = 'LOAD TABLE "' || table_name
     || '" USING FILE ''' || shp_filename || ''' FORMAT SHAPEFILE';
  EXECUTE IMMEDIATE @EXEC_SQL
END;


-- Now do the actual loading
BEGIN
    DECLARE @TABLENAME VARCHAR(50);
    DECLARE @SHAPEFILENAME VARCHAR(200);
    DECLARE @INDEXNAME VARCHAR(50);
    DECLARE @INDEXCOL VARCHAR(80);
    DECLARE @EXEC_SQL TEXT;

    MESSAGE 'Begin loading of QGIS sample shapefiles from directory "' || @QGIS_SAMPLES_DIR || '"' TYPE INFO TO CLIENT;

    -- drop all sample tables if they exist
    FOR droploop AS dropcurs NO SCROLL CURSOR FOR
	SELECT TRIM(row_value) SampleName 
	FROM sa_split_list( @QGIS_SAMPLES_LIST, ',' )
    DO
        SET @TABLENAME = @QGIS_SAMPLES_TABLEPREFIX || SampleName;        
        MESSAGE 'Dropping table ' || @TABLENAME TYPE INFO TO CLIENT;
	SET @EXEC_SQL='DROP TABLE IF EXISTS "' || @TABLENAME || '"';
        EXECUTE IMMEDIATE @EXEC_SQL;
    END FOR;

    -- install pre-defined SRSs and UOMs
    -- EPSG 2967 is not pre-defined, so create it as 1000002967
    MESSAGE 'Creating SRS 1000002967' TYPE INFO TO CLIENT;
    CALL sa_install_feature( 'st_geometry_predefined_srs' );
    CREATE OR REPLACE SPATIAL REFERENCE SYSTEM "NAD27 / Alaska Albers"
        IDENTIFIED BY 1000002964
        LINEAR UNIT OF MEASURE "US survey foot"
        TYPE PLANAR
        COORDINATE X BETWEEN -7401605.9114 AND 5402130.8135
        COORDINATE Y BETWEEN 1376348.1002  AND 8782951.2026
        AXIS ORDER 'x/y/z/m'
        ORGANIZATION 'EPSG' IDENTIFIED BY 2964
        DEFINITION 'PROJCS["Albers Equal Area",
            GEOGCS["NAD27",
            DATUM["North_American_Datum_1927",
            SPHEROID["Clarke 1866",6378206.4,294.978698213898,
            AUTHORITY["EPSG","7008"]],
            TOWGS84[-3,142,183,0,0,0,0],
            AUTHORITY["EPSG","6267"]],
            PRIMEM["Greenwich",0,
            AUTHORITY["EPSG","8901"]],
            UNIT["degree",0.0174532925199433,
            AUTHORITY["EPSG","9108"]],
            AUTHORITY["EPSG","4267"]],
            PROJECTION["Albers_Conic_Equal_Area"],
            PARAMETER["standard_parallel_1",55],
            PARAMETER["standard_parallel_2",65],
            PARAMETER["latitude_of_center",50],
            PARAMETER["longitude_of_center",-154],
            PARAMETER["false_easting",0],
            PARAMETER["false_northing",0],
            UNIT["us_survey_feet",0.3048006096012192]]'
        TRANSFORM DEFINITION '+proj=aea +lat_1=55 +lat_2=65 +lat_0=50 +lon_0=-154 +x_0=0 +y_0=0 +ellps=clrk66 +datum=NAD27 +to_meter=0.3048006096012192 +no_defs'
    ;

    -- create and load tables from the shapefiles
    FOR loadloop AS loadcurs NO SCROLL CURSOR FOR
	SELECT TRIM(row_value) SampleName 
	FROM sa_split_list( @QGIS_SAMPLES_LIST, ',' )
    DO
        SET @TABLENAME = @QGIS_SAMPLES_TABLEPREFIX || SampleName;        
        SET @SHAPEFILENAME = @QGIS_SAMPLES_DIR || SampleName || '.shp';        
        MESSAGE 'Loading table ' || @TABLENAME || ' from "' || @SHAPEFILENAME || '"' TYPE INFO TO CLIENT;
        CALL load_shapefile( @SHAPEFILENAME, 1000002964, @TABLENAME );
    END FOR;

    -- create geometry indexes
    FOR indexloop AS indexcurs NO SCROLL CURSOR FOR
	SELECT TRIM(row_value) SampleName 
	FROM sa_split_list( @QGIS_SAMPLES_LIST, ',' )
    DO
        SET @TABLENAME = @QGIS_SAMPLES_TABLEPREFIX || SampleName;        
        SET @INDEXNAME = @TABLENAME || '_geometry_idx';
	SET @INDEXCOL = @TABLENAME || ' ( geometry )';
	SET @EXEC_SQL = 'CREATE INDEX ' || @INDEXNAME || ' ON ' || @INDEXCOL; 
        MESSAGE 'Creating spatial index ' || @INDEXNAME || ' on ' || @INDEXCOL TYPE INFO TO CLIENT;
        EXECUTE IMMEDIATE @EXEC_SQL;
    END FOR;

    MESSAGE 'Completed loading of QGIS sample shapefiles' TYPE INFO TO CLIENT;
END;


DROP VARIABLE @QGIS_SAMPLES_DIR;
DROP VARIABLE @QGIS_SAMPLES_TABLEPREFIX;
DROP VARIABLE @QGIS_SAMPLES_LIST;
