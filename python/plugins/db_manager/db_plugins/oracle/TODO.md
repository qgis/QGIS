# Won't fix

* SQL Window - "Retrieve columns" don't work => problem in
  dlg_sql_window.py (not enough abstraction for connectors). 

* SQL Window - Can't retrieve column: dlg_sql_window.py launch an
  invalid request for Oracle (no LIMIT operator in Oracle). Should
  patch dlg_sql_window.py to use a connector method to grab columns.

* Query-builder - Add date functions menu (needs to change the main
  code of query builder).

* Table Menu - remove GEOMETRY COLLECTION from the list of geometry
  types (Oracle can't handle this form of geometries). Needs to modify
  dlg_create_table.py
 
* Refresh - Refresh Action should transmit an argument to the
  db_plugin object specifying that this a refresh and not a primary
  dbtree population. I've build (quite) an ugly hack to use SQLite
  list of tables cache for primary population and once it is done,
  each other call to connector.tables doesn't not use cache anymore.

* There is a bug in the create table dialog: there should be a
  checkbox for Null column but it is not showned. The setData method
  don't fix the value for notNull attributes and whatever value of
  this attribute for the field, it is always shown as nullable in the
  modify field dialog. #13089

* Import Table does not work because of a problem in
  QgsVectorLayerImport. After the creation of the table, QGIS tries to
  open the layer but, as there is no geometries in it, QGIS can't
  determinate the geometry type of the layer and refuses to open
  it. Then, no data importation can occur. Must dig into
  src/core/qgsvectorlayerimport.cpp and into the provider code.
  See #13096 .

# Future work

* Implement SRID modification (with reprojection of data) ?
* Modify/force wkbType to be able to open an empty layer in the canvas.

# TODO

* Nothing for the moment.

# DONE

* Code review
  * Try to factorise connector.py.
  * Connector method to handle metadata operations (update/delete/insert).
  * work on updateExtentMetadata.
  * 2.10: try to find the differences between postGIS connector 2.8 and 2.10. (Answer: nothing else than pep-8 reformatting).

* Plugin
  * Can't open spatial view (no pkcol determined).

* Info model
  * Still some problems with extent comparison: do not try to calculate extent when there is no row.
  * Add precision for float values (NUMBER(10,5)).
  * View: show the estimated metadata extent for spatial views that have metadata.
  * Can't rename a spatial View.
  * Can't rename a column.

* Table Tab
  * Problem on one layer casting for number without max_length_char.
  * When exploring some tables in table view: there is a sort of crash with fetchmoreData.

* Query Builder
  * Should not present the geometric columns in column values.

* SQL Window
  * Grab the row count.
  * Create view button.
  * Spatial Index tab (find if can be activated: no because Oracle use spatial index internally).
  * Can't open query builder.
  * Modify dictionary to add math/string/aggregates functions/operators.
  * Can't open column values from query builder.
  * GEOM columns are not retrieved. (Done: don't grab them, just use GEOMETRY string instead to speed columns population)
  * Can't open a query in QGis.
  * Find the srid of a query and add it as a layer parameter.
  * Open non geometric queries.
  * Show better statistic about query duration.
  * handle DatabaseError in the dialog.
  * Make the SQL query window works !
  * Handle geometry type detection.

* Table Menu
  * Error when update/create/delete field in deleteCursor (was problem on QtSql Exceptions).
  * Can't modify the name of a geographic column: modify also the spatial index.
  * Create table: problem with index and geocol name is not in upper case.
  * Create Table: handle 3rd and 4th dimension for metadata creation.
  * Can't modify a field length.
  * Can't delete a geographic column.
  * Add an action to update metadata layer extent to the content of the geocolumn.
  * Disable move Table to another schema (impossible in Oracle, you have to import/export).
  * Find how to load Unknown WKBGeometryType layers.
  * Spatial index creation.
  * Edit dialog fully functionnal.
  * Edit Table opens.
  * Remove Vacuum operation in Table menu.
  * Fix: Add SRID when creating table.
  * Rename Geographic Table.
  * Can create non geographic tables.
  * Can delete non geographic tables.
  * Can Create geographic tables.
  * Can Delete geographic tables.
  * DO Empty table.

* Connector
  * getVectorTablesCache should present only one line per table (don't show tables more than once).
  * correspondance between Oracle SDO_GTYPE and QGis.WkbType in a class dict.
  * Show only one table/layer in the table tree even if entry is duplicated in cache. 
  * Reuse the pre-calculated extent metadata when updating.
  * Create a better privilege function using ALL_PRIVS_RECD

* Main Tree
  * Bug on refreshing a whole connection.

* Information Tab (main tab)
  * If extent is not in QGIS format (dimensions named X and Y): show it and offer to correct the name of the dimension.
  * Syntax coloration for View and MView definitions (done with pygments which is included in QGIS).
  * Add date of last creation/modification for objects.
  * Modify "Oracle" in table information by "Oracle Spatial".
  * For rows estimation: use a query to scan nb_rows in object !
  * Refresh on the fly data that are not stored in cache (comments/table type/row counts).
  * Bad comparison between metadata extent and calculated extent (was format syntax).
  * If user can't udpate metadata, don't pull the link for update action.
  * Can't update metadata extent: find if table is in USER_SDO_GEOM_METADATA.
  * Print QGis Geometry Type in string instead of int.

* Constraints: 
  * Handle foreign keys information.
  * Add search_condition, status, delete_rule, validated, generated

* Indexes:
  * Add rebuild index action for indexes.
  * Add more information: Compression/Valid/Last analyzed/ityp_name.

* Refresh:
  * Make getTables and getVectorTables functions retrieve less information and manage them on the fly.
  * Refreshing a schema refresh only the schema, not the whole connection.	
  * Still problems with refreshing (QPyNullVariant for detectedSrid).
  * Mark views as views.
  * Grab also geographic views.
  * Refresh should launch a refresh of the cached list of tables (SQLite).
  * Implement add_sys_tables (mask MDRT index tables for example).
  * List of unknown geometry tables when refreshing depending upon onlyExistingTypes.
  * Grab the geomtypes of the layer during list refreshing.
  * Grab the srids of the layer during list refreshing.
  * Grab the pkcols for the views.
  * Bug with cache srids interpreted as float (convert detected srid to int).
  * Find if a layer is a view or a table with cache and non cache list of tables.

* Materialized views:
  * function to determine if MV or simple Table.
  * retrieve definition.
  * unitary comments retrieving.
  * Primary key mask.
  * Refreshing information.
  * Action to refresh the mview.
  * View/MView definition is not interpreted as Html anymore.

* QtSqlDB: 
  * bug with timing functions (no Time module/global name)
  * better handling of dates (returned as QDateTime) in query dialog.
