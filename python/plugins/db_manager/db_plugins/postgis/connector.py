# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on 
- PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ..connector import DBConnector
from ..plugin import ConnectionError, DbError, Table

import psycopg2
import psycopg2.extensions
# use unicode!
psycopg2.extensions.register_type(psycopg2.extensions.UNICODE)
psycopg2.extensions.register_type(psycopg2.extensions.UNICODEARRAY)


def classFactory():
	return PostGisDBConnector

class PostGisDBConnector(DBConnector):
	def __init__(self, uri):
		DBConnector.__init__(self, uri)

		self.host = uri.host()
		self.port = uri.port()
		self.dbname = uri.database()
		self.user = uri.username()
		self.passwd = uri.password()
				
		if self.dbname == '' or self.dbname is None:
			self.dbname = self.user
		
		try:
			self.connection = psycopg2.connect( self._connectionInfo().encode('utf-8') )
		except self.connection_error_types(), e:
			raise ConnectionError(e)
		
		self._checkSpatial()
		self._checkRaster()
		self._checkGeometryColumnsTable()
		self._checkRasterColumnsTable()

	def _connectionInfo(self):
		return unicode(self._uri.connectionInfo())

	def _checkSpatial(self):
		""" check whether postgis_version is present in catalog """
		c = self._get_cursor()
		self._execute(c, u"SELECT COUNT(*) FROM pg_proc WHERE proname = 'postgis_version'")
		self.has_spatial = c.fetchone()[0] > 0
		return self.has_spatial
	
	def _checkRaster(self):
		""" check whether postgis_version is present in catalog """
		c = self._get_cursor()
		self._execute(c, u"SELECT COUNT(*) FROM pg_proc WHERE proname = 'postgis_raster_lib_version'")
		self.has_raster = c.fetchone()[0] > 0
		return self.has_raster
	
	def _checkGeometryColumnsTable(self):
		c = self._get_cursor()
		self._execute(c, u"SELECT relkind = 'v' FROM pg_class WHERE relname = 'geometry_columns' AND relkind IN ('v', 'r')")
		res = c.fetchone()
		self.has_geometry_columns = (res != None and len(res) != 0)
		
		if not self.has_geometry_columns:
			self.has_geometry_columns_access = self.is_geometry_columns_view = False
		else:
			self.is_geometry_columns_view = res[0]
			# find out whether has privileges to access geometry_columns table
			priv = self.getTablePrivileges('geometry_columns')
			self.has_geometry_columns_access = priv[0]
		return self.has_geometry_columns

	def _checkRasterColumnsTable(self):
		c = self._get_cursor()
		self._execute(c, u"SELECT relkind = 'v' FROM pg_class WHERE relname = 'raster_columns' AND relkind IN ('v', 'r')")
		res = c.fetchone()
		self.has_raster_columns = (res != None and len(res) != 0)
		
		if not self.has_raster_columns:
			self.has_raster_columns_access = self.is_raster_columns_view = False
		else:
			self.is_raster_columns_view = res[0]
			# find out whether has privileges to access geometry_columns table
			self.has_raster_columns_access = self.getTablePrivileges('raster_columns')[0]
		return self.has_raster_columns

	def getInfo(self):
		c = self._get_cursor()
		self._execute(c, u"SELECT version()")
		return c.fetchone()

	def getSpatialInfo(self):
		""" returns tuple about postgis support:
			- lib version
			- installed scripts version
			- released scripts version
			- geos version
			- proj version
			- whether uses stats
		"""
		if not self.has_spatial:
			return

		c = self._get_cursor()
		try:
			self._execute(c, u"SELECT postgis_lib_version(), postgis_scripts_installed(), postgis_scripts_released(), postgis_geos_version(), postgis_proj_version(), postgis_uses_stats()")
		except DbError:
			return

		return c.fetchone()

	def hasSpatialSupport(self):
		return self.has_spatial

	def hasRasterSupport(self):
		return self.has_raster

	def hasCustomQuerySupport(self):
		from qgis.core import QGis
		return QGis.QGIS_VERSION[0:3] >= "1.5"

	def hasTableColumnEditingSupport(self):
		return True


	def fieldTypes(self):
		return [
			"integer", "bigint", "smallint", # integers
			"serial", "bigserial", # auto-incrementing ints
			"real", "double precision", "numeric", # floats
			"varchar", "varchar(n)", "char(n)", "text", # strings
			"date", "time", "timestamp" # date/time
		]



	def getDatabasePrivileges(self):
		""" db privileges: (can create schemas, can create temp. tables) """
		sql = u"SELECT has_database_privilege(%(d)s, 'CREATE'), has_database_privilege(%(d)s, 'TEMP')" % { 'd' : self.quoteString(self.dbname) }
		c = self._get_cursor()
		self._execute(c, sql)
		return c.fetchone()
		
	def getSchemaPrivileges(self, schema):
		""" schema privileges: (can create new objects, can access objects in schema) """
		schema = 'current_schema()' if schema == None else self.quoteString(schema)
		sql = u"SELECT has_schema_privilege(%(s)s, 'CREATE'), has_schema_privilege(%(s)s, 'USAGE')" % { 's' : schema }
		c = self._get_cursor()
		self._execute(c, sql)
		return c.fetchone()
	
	def getTablePrivileges(self, table):
		""" table privileges: (select, insert, update, delete) """

		schema, tablename = self.getSchemaTableName(table)
		schema_priv = self.getSchemaPrivileges(schema)
		if not schema_priv[1]:
			return

		t = self.quoteId( table )
		sql = u"""SELECT has_table_privilege(%(t)s, 'SELECT'), has_table_privilege(%(t)s, 'INSERT'),
		                has_table_privilege(%(t)s, 'UPDATE'), has_table_privilege(%(t)s, 'DELETE')""" % { 't': self.quoteString(t) }
		c = self._get_cursor()
		self._execute(c, sql)
		return c.fetchone()


	def getSchemas(self):
		""" get list of schemas in tuples: (oid, name, owner, perms) """
		c = self._get_cursor()
		sql = u"SELECT oid, nspname, pg_get_userbyid(nspowner), nspacl, pg_catalog.obj_description(oid) FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema' ORDER BY nspname"
		self._execute(c, sql)
		return c.fetchall()

	def getTables(self, schema=None):
		""" get list of tables """
		tablenames = []
		items = []

		sys_tables = [ "spatial_ref_sys", "geography_columns", "geometry_columns", 
				"raster_columns", "raster_overviews" ]

		try:
			vectors = self.getVectorTables(schema)
			for tbl in vectors:
				if tbl[1] in sys_tables and tbl[2] in ['', 'public']:
					continue
				tablenames.append( (tbl[2], tbl[1]) )
				items.append( tbl )
		except DbError:
			pass

		try:
			rasters = self.getRasterTables(schema)
			for tbl in rasters:
				if tbl[1] in sys_tables and tbl[2] in ['', 'public']:
					continue
				tablenames.append( (tbl[2], tbl[1]) )
				items.append( tbl )
		except DbError:
			pass

		c = self._get_cursor()

		sys_tables = [ "spatial_ref_sys", "geography_columns", "geometry_columns", 
				"raster_columns", "raster_overviews" ]
		
		if schema:
			schema_where = u" AND nspname = %s " % self.quoteString(schema)
		else:
			schema_where = u" AND (nspname != 'information_schema' AND nspname !~ 'pg_') "
			
		# get all tables and views
		sql = u"""SELECT 
						cla.relname, nsp.nspname, cla.relkind = 'v', 
						pg_get_userbyid(relowner), reltuples, relpages, 
						pg_catalog.obj_description(cla.oid)
					FROM pg_class AS cla 
					JOIN pg_namespace AS nsp ON nsp.oid = cla.relnamespace
					WHERE cla.relkind IN ('v', 'r') """ + schema_where + """
					ORDER BY nsp.nspname, cla.relname"""
						  
		self._execute(c, sql)

		for tbl in c.fetchall():
			if tablenames.count( (tbl[1], tbl[0]) ) <= 0:
				item = list(tbl)
				item.insert(0, Table.TableType)
				items.append( item )

		return sorted( items, cmp=lambda x,y: cmp((x[2],x[1]), (y[2],y[1])) )


	def getVectorTables(self, schema=None):
		""" get list of table with a geometry column
			it returns:
				name (table name)
				namespace (schema)
				type = 'view' (is a view?)
				owner 
				tuples
				pages
				geometry_column:
					f_geometry_column (or pg_attribute.attname, the geometry column name)
					type (or pg_attribute.atttypid::regtype, the geometry column type name)
					coord_dimension 
					srid
		"""

		if not self.has_spatial:
			return []

		c = self._get_cursor()
		
		if schema:
			schema_where = u" AND nspname = %s " % self.quoteString(schema)
		else:
			schema_where = u" AND (nspname != 'information_schema' AND nspname !~ 'pg_') "

		geometry_column_from = u""
		geometry_fields_select = u"""att.attname, 
						textin(regtypeout(att.atttypid::regtype)), 
						NULL, NULL"""
		if self.has_geometry_columns and self.has_geometry_columns_access:
			geometry_column_from = u"""LEFT OUTER JOIN geometry_columns AS geo ON 
						cla.relname = geo.f_table_name AND nsp.nspname = f_table_schema AND 
						lower(att.attname) = lower(f_geometry_column)"""
			geometry_fields_select = u"""CASE WHEN geo.f_geometry_column IS NOT NULL THEN geo.f_geometry_column ELSE att.attname END, 
						CASE WHEN geo.type IS NOT NULL THEN geo.type ELSE textin(regtypeout(att.atttypid::regtype)) END, 
						geo.coord_dimension, geo.srid"""
			

		# discovery of all tables and whether they contain a geometry column
		sql = u"""SELECT 
						cla.relname, nsp.nspname, cla.relkind = 'v', 
						pg_get_userbyid(relowner), cla.reltuples, cla.relpages, 
						pg_catalog.obj_description(cla.oid), 
						""" + geometry_fields_select + """

					FROM pg_class AS cla 
					JOIN pg_namespace AS nsp ON 
						nsp.oid = cla.relnamespace

					JOIN pg_attribute AS att ON 
						att.attrelid = cla.oid AND 
						att.atttypid = 'geometry'::regtype OR 
						att.atttypid IN (SELECT oid FROM pg_type WHERE typbasetype='geometry'::regtype ) 

					""" + geometry_column_from + """ 

					WHERE cla.relkind IN ('v', 'r') """ + schema_where + """ 
					ORDER BY nsp.nspname, cla.relname, att.attname"""

		self._execute(c, sql)

		items = []
		for i, tbl in enumerate(c.fetchall()):
			item = list(tbl)
			item.insert(0, Table.VectorType)
			items.append( item )
			
		return items

	def getRasterTables(self, schema=None):
		""" get list of table with a raster column
			it returns:
				name (table name)
				namespace (schema)
				type = 'view' (is a view?)
				owner 
				tuples
				pages
				raster_column:
					r_raster_column (or pg_attribute.attname, the raster column name)
					pixel type
					block size
					internal or external
					srid
		"""

		if not self.has_spatial:
			return []
		if not self.has_raster:
			return []

		c = self._get_cursor()
		
		if schema:
			schema_where = u" AND nspname = %s " % self.quoteString(schema)
		else:
			schema_where = u" AND (nspname != 'information_schema' AND nspname !~ 'pg_') "

		raster_column_from = u""
		raster_fields_select = u"""att.attname, NULL, NULL, NULL, NULL, NULL"""
		if self.has_raster_columns and self.has_raster_columns_access:
			raster_column_from = u"""LEFT OUTER JOIN raster_columns AS rast ON 
						cla.relname = rast.r_table_name AND nsp.nspname = r_table_schema AND 
						lower(att.attname) = lower(r_raster_column)"""
			raster_fields_select = u"""CASE WHEN rast.r_raster_column IS NOT NULL THEN rast.r_raster_column ELSE att.attname END, 
						rast.pixel_types,
						rast.scale_x,
						rast.scale_y,
						rast.out_db,
						rast.srid"""


		# discovery of all tables and whether they contain a raster column
		sql = u"""SELECT 
						cla.relname, nsp.nspname, cla.relkind = 'v', 
						pg_get_userbyid(relowner), cla.reltuples, cla.relpages, 
						pg_catalog.obj_description(cla.oid), 
						""" + raster_fields_select + """

					FROM pg_class AS cla 
					JOIN pg_namespace AS nsp ON 
						nsp.oid = cla.relnamespace

					JOIN pg_attribute AS att ON 
						att.attrelid = cla.oid AND 
						att.atttypid = 'raster'::regtype OR 
						att.atttypid IN (SELECT oid FROM pg_type WHERE typbasetype='raster'::regtype ) 

					""" + raster_column_from + """ 

					WHERE cla.relkind IN ('v', 'r') """ + schema_where + """ 
					ORDER BY nsp.nspname, cla.relname, att.attname"""

		self._execute(c, sql)

		items = []
		for i, tbl in enumerate(c.fetchall()):
			item = list(tbl)
			item.insert(0, Table.RasterType)
			items.append( item )
			
		return items


	def getTableRowCount(self, table):
		c = self._get_cursor()
		self._execute( c, u"SELECT COUNT(*) FROM %s" % self.quoteId(table) )
		return c.fetchone()[0]

	def getTableFields(self, table):
		""" return list of columns in table """
		c = self._get_cursor()

		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" AND nspname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT a.attnum AS ordinal_position,
				a.attname AS column_name,
				t.typname AS data_type,
				a.attlen AS char_max_len,
				a.atttypmod AS modifier,
				a.attnotnull AS notnull,
				a.atthasdef AS hasdefault,
				adef.adsrc AS default_value, 
				pg_catalog.format_type(a.atttypid,a.atttypmod) AS formatted_type
			FROM pg_class c
			JOIN pg_attribute a ON a.attrelid = c.oid
			JOIN pg_type t ON a.atttypid = t.oid
			JOIN pg_namespace nsp ON c.relnamespace = nsp.oid
			LEFT JOIN pg_attrdef adef ON adef.adrelid = a.attrelid AND adef.adnum = a.attnum
			WHERE
			  a.attnum > 0 AND c.relname=%s %s
			ORDER BY a.attnum""" % (self.quoteString(tablename), schema_where)

		self._execute(c, sql)
		return c.fetchall()

	def getTableIndexes(self, table):
		""" get info about table's indexes. ignore primary key constraint index, they get listed in constaints """
		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" AND nspname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT idxcls.relname, indkey, indisunique = 't' 
						FROM pg_index JOIN pg_class ON pg_index.indrelid=pg_class.oid 
						JOIN pg_class AS idxcls ON pg_index.indexrelid=idxcls.oid 
						JOIN pg_namespace nsp ON pg_class.relnamespace = nsp.oid 
							WHERE pg_class.relname=%s %s 
							AND indisprimary != 't' """ % (self.quoteString(tablename), schema_where)
		c = self._get_cursor()
		self._execute(c, sql)
		return c.fetchall()
	
	
	def getTableConstraints(self, table):
		c = self._get_cursor()
		
		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" AND nspname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT c.conname, c.contype, c.condeferrable, c.condeferred, array_to_string(c.conkey, ' '), c.consrc,
		         t2.relname, c.confupdtype, c.confdeltype, c.confmatchtype, array_to_string(c.confkey, ' ') FROM pg_constraint c
		  LEFT JOIN pg_class t ON c.conrelid = t.oid
			LEFT JOIN pg_class t2 ON c.confrelid = t2.oid
			JOIN pg_namespace nsp ON t.relnamespace = nsp.oid
			WHERE t.relname = %s %s """ % (self.quoteString(tablename), schema_where)
		
		self._execute(c, sql)
		return c.fetchall()


	def getTableTriggers(self, table):
		c = self._get_cursor()
		
		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" AND nspname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT tgname, proname, tgtype, tgenabled NOT IN ('f', 'D')  FROM pg_trigger trig
		          LEFT JOIN pg_class t ON trig.tgrelid = t.oid
							LEFT JOIN pg_proc p ON trig.tgfoid = p.oid
							JOIN pg_namespace nsp ON t.relnamespace = nsp.oid
							WHERE t.relname = %s %s """ % (self.quoteString(tablename), schema_where)
	
		self._execute(c, sql)
		return c.fetchall()

	def enableAllTableTriggers(self, enable, table):
		""" enable or disable all triggers on table """
		self.enableTableTrigger(None, enable, table)
		
	def enableTableTrigger(self, trigger, enable, table):
		""" enable or disable one trigger on table """
		trigger = self.quoteId(trigger) if trigger != None else "ALL"
		sql = u"ALTER TABLE %s %s TRIGGER %s" % (self.quoteId(table), "ENABLE" if enable else "DISABLE", trigger)
		self._execute_and_commit(sql)
		
	def deleteTableTrigger(self, trigger, table):
		""" delete trigger on table """
		sql = u"DROP TRIGGER %s ON %s" % (self.quoteId(trigger), self.quoteId(table))
		self._execute_and_commit(sql)
		
	
	def getTableRules(self, table):
		c = self._get_cursor()
		
		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" AND schemaname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT rulename, definition FROM pg_rules
					WHERE tablename=%s %s """ % (self.quoteString(tablename), schema_where)
	
		self._execute(c, sql)
		return c.fetchall()

	def deleteTableRule(self, rule, table):
		""" delete rule on table """
		sql = u"DROP RULE %s ON %s" % (self.quoteId(rule), self.quoteId(table))
		self._execute_and_commit(sql)


	def getTableExtent(self, table, geom):
		""" find out table extent """
		c = self._get_cursor()
		subquery = u"SELECT st_extent(%s) AS extent FROM %s" % ( self.quoteId(geom), self.quoteId(table) )
		sql = u"SELECT st_xmin(extent), st_ymin(extent), st_xmax(extent), st_ymax(extent) FROM (%s) AS subquery" % subquery
		self._execute(c, sql)
		return c.fetchone()

	def getTableEstimatedExtent(self, table, geom):
		""" find out estimated extent (from the statistics) """
		if self.isRasterTable(table):
			return

		c = self._get_cursor()
		schema, tablename = self.getSchemaTableName(table)
		schema_part = u"%s," % self.quoteString(schema) if schema is not None else ""

		subquery = u"SELECT st_estimated_extent(%s%s,%s) AS extent" % (schema_part, self.quoteString(tablename), self.quoteString(geom))
		sql = u"""SELECT st_xmin(extent), st_ymin(extent), st_xmax(extent), st_ymax(extent) FROM (%s) AS subquery """ % subquery
		try:
			self._execute(c, sql)
		except DbError, e:	# no statistics for the current table
			return
		return c.fetchone()
	
	def getViewDefinition(self, view):
		""" returns definition of the view """

		schema, tablename = self.getSchemaTableName(view)
		schema_where = u" AND nspname=%s " % self.quoteString(schema) if schema is not None else ""

		sql = u"""SELECT pg_get_viewdef(c.oid) FROM pg_class c
						JOIN pg_namespace nsp ON c.relnamespace = nsp.oid
		        WHERE relname=%s %s AND relkind='v' """ % (self.quoteString(tablename), schema_where)
		c = self._execute(None, sql)
		res = c.fetchone()
		return res[0] if res is not None else None

	def getSpatialRefInfo(self, srid):
		if not self.has_spatial:
			return
		
		try:
			c = self._get_cursor()
			self._execute(c, "SELECT srtext FROM spatial_ref_sys WHERE srid = '%d'" % srid)
			sr = c.fetchone()
			if sr == None:
				return
			srtext = sr[0]
			# try to extract just SR name (should be quoted in double quotes)
			regex = QRegExp( '"([^"]+)"' )
			if regex.indexIn( srtext ) > -1:
				srtext = regex.cap(1)
			return srtext
		except DbError, e:
			return


	def isVectorTable(self, table):
		if self.has_geometry_columns and self.has_geometry_columns_access:
			schema, tablename = self.getSchemaTableName(table)
			sql = u"SELECT count(*) FROM geometry_columns WHERE f_table_schema = %s AND f_table_name = %s" % (self.quoteString(schema), self.quoteString(tablename))
			c = self._execute(None, sql)
			res = c.fetchone()
			return res != None and res[0] > 0
		return False

	def isRasterTable(self, table):
		if self.has_raster_columns and self.has_raster_columns_access:
			schema, tablename = self.getSchemaTableName(table)
			sql = u"SELECT count(*) FROM raster_columns WHERE r_table_schema = %s AND r_table_name = %s" % (self.quoteString(schema), self.quoteString(tablename))
			c = self._execute(None, sql)
			res = c.fetchone()
			return res != None and res[0] > 0
		return False		


	def createTable(self, table, field_defs, pkey):
		""" create ordinary table
				'fields' is array containing field definitions
				'pkey' is the primary key name
		"""
		if len(field_defs) == 0:
			return False

		sql = "CREATE TABLE %s (" % self.quoteId(table)
		sql += u", ".join( field_defs )
		if pkey != None and pkey != "":
			sql += u", PRIMARY KEY (%s)" % self.quoteId(pkey)
		sql += ")"

		self._execute_and_commit(sql)
		return True

	def deleteTable(self, table):
		""" delete table and its reference in either geometry_columns or raster_columns """
		schema, tablename = self.getSchemaTableName(table)
		schema_part = u"%s, " % self.quoteString(schema) if schema is not None else ""
		if self.isVectorTable(table):
			sql = u"SELECT DropGeometryTable(%s%s)" % (schema_part, self.quoteString(tablename))
		elif self.isRasterTable(table):
			sql = u"SELECT DropRasterTable(%s%s)" % (schema_part, self.quoteString(tablename))
		else:
			sql = u"DROP TABLE %s" % self.quoteId(table)
		self._execute_and_commit(sql)


	def emptyTable(self, table):
		""" delete all rows from table """
		sql = u"TRUNCATE %s" % self.quoteId(table)
		self._execute_and_commit(sql)

	def renameTable(self, table, new_table):
		""" rename a table in database """
		schema, tablename = self.getSchemaTableName(table)
		if new_table == tablename:
			return
		c = self._get_cursor()

		sql = u"ALTER TABLE %s RENAME TO %s" % (self.quoteId(table), self.quoteId(new_table))
		self._execute(c, sql)
		
		# update geometry_columns if postgis is enabled
		if self.has_geometry_columns and not self.is_geometry_columns_view:
			schema_where = u" AND f_table_schema=%s " % self.quoteString(schema) if schema is not None else ""
			sql = u"UPDATE geometry_columns SET f_table_name=%s WHERE f_table_name=%s %s" % (self.quoteString(new_table), self.quoteString(tablename), schema_where)
			self._execute(c, sql)

		self._commit()

	def moveTableToSchema(self, table, new_schema):
		schema, tablename = self.getSchemaTableName(table)
		if new_schema == schema:
			return
		c = self._get_cursor()

		sql = u"ALTER TABLE %s SET SCHEMA %s" % (self.quoteId(table), self.quoteId(new_schema))
		self._execute(c, sql)
		
		# update geometry_columns if postgis is enabled
		if self.has_geometry_columns and not self.is_geometry_columns_view:
			schema, tablename = self.getSchemaTableName(table)
			schema_where = u" AND f_table_schema=%s " % self.quoteString(schema) if schema is not None else ""
			sql = u"UPDATE geometry_columns SET f_table_schema=%s WHERE f_table_name=%s %s" % (self.quoteString(new_schema), self.quoteString(tablename), schema_where)
			self._execute(c, sql)

		self._commit()

	def moveTable(self, table, new_table, new_schema=None):
		schema, tablename = self.getSchemaTableName(table)
		if new_schema == schema and new_table == tablename: 
			return
		if new_schema == schema:
			return self.renameTable(table, new_table)
		if new_table == table:
			return self.moveTableToSchema(table, new_schema)

		c = self._get_cursor()
		t = u"__new_table__"

		sql = u"ALTER TABLE %s RENAME TO %s" % (self.quoteId(table), self.quoteId(t))
		self._execute(c, sql)

		sql = u"ALTER TABLE %s SET SCHEMA %s" % (self.quoteId( (schema, t) ), self.quoteId(new_schema))
		self._execute(c, sql)

		sql = u"ALTER TABLE %s RENAME TO %s" % (self.quoteId( (new_schema, t) ), self.quoteId(table))
		self._execute(c, sql)

		# update geometry_columns if postgis is enabled
		if self.has_geometry_columns and not self.is_geometry_columns_view:
			schema, tablename = self.getSchemaTableName(table)
			schema_where = u" f_table_schema=%s AND " % self.quoteString(schema) if schema is not None else ""
			schema_part = u" f_table_schema=%s, " % self.quoteString(new_schema) if schema is not None else ""
			sql = u"UPDATE geometry_columns SET %s f_table_name=%s WHERE %s f_table_name=%s" % (schema_part, self.quoteString(new_table), schema_where, self.quoteString(tablename))
			self._execute(c, sql)

		self._commit()
		
	def createView(self, view, query):
		sql = u"CREATE VIEW %s AS %s" % (self.quoteId(view), query)
		self._execute_and_commit(sql)
	
	def deleteView(self, view):
		sql = u"DROP VIEW %s" % self.quoteId(view)
		self._execute_and_commit(sql)
	
	def renameView(self, view, new_name):
		""" rename view in database """
		self.renameTable(view, new_name)
		
	def createSchema(self, schema):
		""" create a new empty schema in database """
		sql = u"CREATE SCHEMA %s" % self.quoteId(schema)
		self._execute_and_commit(sql)
		
	def deleteSchema(self, schema):
		""" drop (empty) schema from database """
		sql = u"DROP SCHEMA %s" % self.quoteId(schema)
		self._execute_and_commit(sql)
		
	def renameSchema(self, schema, new_schema):
		""" rename a schema in database """
		sql = u"ALTER SCHEMA %s RENAME TO %s" % (self.quoteId(schema), self.quoteId(new_schema))
		self._execute_and_commit(sql)


	def runVacuum(self):
		""" run vacuum on the db """
		self._execute_and_commit("VACUUM")

	def runVacuumAnalyze(self, table):
		""" run vacuum analyze on a table """
		# vacuum analyze must be run outside transaction block - we have to change isolation level
		self.connection.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)
		c = self._get_cursor()
		sql = u"VACUUM ANALYZE %s" % self.quoteId(table)
		self._execute(c, sql)
		self.connection.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_READ_COMMITTED)


	def addTableColumn(self, table, field_def):
		""" add a column to table """
		sql = u"ALTER TABLE %s ADD %s" % (self.quoteId(table), field_def)
		self._execute_and_commit(sql)
		
	def deleteTableColumn(self, table, column):
		""" delete column from a table """
		if self.isGeometryColumn(table, column):
			# use postgis function to delete geometry column correctly
			schema, tablename = self.getSchemaTableName(table)
			schema_part = u"%s, " % self._quote_str(schema) if schema else ""
			sql = u"SELECT DropGeometryColumn(%s%s, %s)" % (schema_part, self.quoteString(tablename), self.quoteString(column))
		else:
			sql = u"ALTER TABLE %s DROP %s" % (self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)

	def updateTableColumn(self, table, column, new_name=None, data_type=None, not_null=None, default=None):
		if new_name == None and data_type == None and not_null == None and default == None:
			return

		c = self._get_cursor()

		# update column definition
		col_actions = QStringList()
		if data_type != None:
			col_actions << u"TYPE %s" % data_type
		if not_null != None: 
			col_actions << (u"SET NOT NULL" if not_null else u"DROP NOT NULL")
		if default != None:
			if default and default != '':
				col_actions << u"SET DEFAULT %s" % default
			else:
				col_actions << u"DROP DEFAULT"
		if len(col_actions) > 0:
			sql = u"ALTER TABLE %s" % self.quoteId(table)
			alter_col_str = u"ALTER %s" % self.quoteId(column)
			for a in col_actions:
				sql += u" %s %s," % (alter_col_str, a)
			self._execute(c, sql[:-1])

		# rename the column
		if new_name != None and new_name != column:
			sql = u"ALTER TABLE %s RENAME %s TO %s" % (self.quoteId(table), self.quoteId(column), self.quoteId(new_name))
			self._execute(c, sql)

			# update geometry_columns if postgis is enabled
			if self.has_geometry_columns and not self.is_geometry_columns_view:
				schema, tablename = self.getSchemaTableName(table)
				schema_where = u" f_table_schema=%s AND " % self.quoteString(schema) if schema is not None else ""
				sql = u"UPDATE geometry_columns SET f_geometry_column=%s WHERE %s f_table_name=%s AND f_geometry_column=%s" % (self.quoteString(new_name), schema_where, self.quoteString(tablename), self.quoteString(name))
				self._execute(c, sql)

		self._commit()

	def renameTableColumn(self, table, column, new_name):
		""" rename column in a table """
		return self.updateTableColumn(table, column, new_name)
		
	def setTableColumnType(self, table, column, data_type):
		""" change column type """
		return self.updateTableColumn(table, column, None, data_type)
		
	def setTableColumnNull(self, table, column, is_null):
		""" change whether column can contain null values """
		return self.updateTableColumn(table, column, None, None, not is_null)

	def setTableColumnDefault(self, table, column, default):
		""" change column's default value. 
			If default=None or an empty string drop default value """
		return self.updateTableColumn(table, column, None, None, None, default)


	def isGeometryColumn(self, table, column):
		c = self._get_cursor()

		schema, tablename = self.getSchemaTableName(table)
		schema_where = u" f_table_schema=%s AND " % self.quoteString(schema) if schema is not None else ""

		sql = u"SELECT count(*) > 0 FROM geometry_columns WHERE %s f_table_name=%s AND f_geometry_column=%s" % (schema_where, self.quoteString(tablename), self.quoteString(column))
		self._execute(c, sql)
		return c.fetchone()[0] == 't'

	def addGeometryColumn(self, table, geom_column='geom', geom_type='POINT', srid=-1, dim=2):
		schema, tablename = self.getSchemaTableName(table)
		schema_part = u"%s, " % self.quoteString(schema) if schema else ""

		sql = u"SELECT AddGeometryColumn(%s%s, %s, %d, %s, %d)" % (schema_part, self.quoteString(tablename), self.quoteString(geom_column), srid, self.quoteString(geom_type), dim)
		self._execute_and_commit(sql)

	def deleteGeometryColumn(self, table, geom_column):
		return self.deleteTableColumn(table, geom_column)


	def addTableUniqueConstraint(self, table, column):
		""" add a unique constraint to a table """
		sql = u"ALTER TABLE %s ADD UNIQUE (%s)" % (self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)

	def deleteTableConstraint(self, table, constraint):
		""" delete constraint in a table """
		sql = u"ALTER TABLE %s DROP CONSTRAINT %s" % (self.quoteId(table), self.quoteId(constraint))
		self._execute_and_commit(sql)

	def addTablePrimaryKey(self, table, column):
		""" add a primery key (with one column) to a table """
		sql = u"ALTER TABLE %s ADD PRIMARY KEY (%s)" % (self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)


	def createTableIndex(self, table, name, column):
		""" create index on one column using default options """
		sql = u"CREATE INDEX %s ON %s (%s)" % (self.quoteId(name), self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)
		
	def deleteTableIndex(self, table, name):
		schema, tablename = self.getSchemaTableName(table)
		sql = u"DROP INDEX %s" % self.quoteId( (schema, name) )
		self._execute_and_commit(sql)

	def createSpatialIndex(self, table, geom_column='geom'):
		schema, tablename = self.getSchemaTableName(table)
		idx_name = self.quoteId(u"sidx_%s_%s" % (tablename, geom_column))
		sql = u"CREATE INDEX %s ON %s USING GIST(%s)" % (idx_name, self.quoteId(table), self.quoteId(geom_column))
		self._execute_and_commit(sql)

	def deleteSpatialIndex(self, table, geom_column='geom'):
		schema, tablename = self.getSchemaTableName(table)
		idx_name = self.quoteId(u"sidx_%s_%s" % (tablename, geom_column))
		return self.dropTableIndex(table, idx_name)


	def execution_error_types(self):
		return psycopg2.Error, psycopg2.ProgrammingError

	def connection_error_types(self):
		return psycopg2.InterfaceError, psycopg2.OperationalError

	# moved into the parent class: DbConnector._execute()
	#def _execute(self, cursor, sql):
	#	pass

	# moved into the parent class: DbConnector._execute_and_commit()
	#def _execute_and_commit(self, sql):
	#	pass

	# moved into the parent class: DbConnector._get_cursor()
	#def _get_cursor(self, name=None):
	#	pass

	# moved into the parent class: DbConnector._fetchall()
	#def _fetchall(self, c):
	#	pass

	# moved into the parent class: DbConnector._fetchone()
	#def _fetchone(self, c):
	#	pass

	# moved into the parent class: DbConnector._commit()
	#def _commit(self):
	#	pass

	# moved into the parent class: DbConnector._rollback()
	#def _rollback(self):
	#	pass

	# moved into the parent class: DbConnector._get_cursor_columns()
	#def _get_cursor_columns(self, c):
	#	pass

	def getSqlDictionary(self):
		from .sql_dictionary import getSqlDictionary
		sql_dict = getSqlDictionary()

		# get schemas, tables and field names
		items = []
		sql = u"""SELECT nspname FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema' 
UNION SELECT relname FROM pg_class WHERE relkind IN ('v', 'r') 
UNION SELECT attname FROM pg_attribute WHERE attnum > 0"""
		c = self._execute(None, sql)
		for row in c.fetchall():
			items.append( row[0] )

		sql_dict["identifier"] = items
		return sql_dict

