# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

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

from pyspatialite import dbapi2 as sqlite

def classFactory():
	return SpatiaLiteDBConnector

class SpatiaLiteDBConnector(DBConnector):
	def __init__(self, uri):
		DBConnector.__init__(self, uri)

		self.dbname = uri.database()
		if not QFile.exists( self.dbname ):
			raise ConnectionError( u'"%s" not found' % self.dbname )

		try:
			self.connection = sqlite.connect( self._connectionInfo() )

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		self._checkSpatial()
		self._checkRaster()
		self._checkGeometryColumnsTable()
		self._checkRastersTable()

	def _connectionInfo(self):
		return unicode(self.dbname)
	
	def _checkSpatial(self):
		""" check if it's a valid spatialite db """
		self.has_spatial = self._checkGeometryColumnsTable()
		return self.has_spatial

	def _checkRaster(self):
		""" check if it's a rasterite db """
		self.has_raster = self._checkRastersTable()
		return self.has_raster

	def _checkGeometryColumnsTable(self):
		try:
			c = self._get_cursor()
			self._execute(c, u"SELECT CheckSpatialMetaData()")
			self.has_geometry_columns = c.fetchone()[0] == 1
		except Exception, e:
			self.has_geometry_columns = False

		self.has_geometry_columns_access = self.has_geometry_columns
		return self.has_geometry_columns

	def _checkRastersTable(self):
		c = self._get_cursor()
		sql = u"SELECT count(*) = 3 FROM sqlite_master WHERE name IN ('layer_params', 'layer_statistics', 'raster_pyramids')"
		self._execute(c, sql)
		ret = c.fetchone()
		return ret and ret[0]
	
	def getInfo(self):
		c = self._get_cursor()
		self._execute(c, u"SELECT sqlite_version()")
		return c.fetchone()

	def getSpatialInfo(self):
		""" returns tuple about spatialite support:
			- lib version
			- geos version
			- proj version
		"""
		if not self.has_spatial:
			return

		c = self._get_cursor()
		try:
			self._execute(c, u"SELECT spatialite_version(), geos_version(), proj4_version()")
		except DbError:
			return

		return c.fetchone()

	def hasSpatialSupport(self):
		return self.has_spatial

	def hasRasterSupport(self):
		return self.has_raster

	def hasCustomQuerySupport(self):
		from qgis.core import QGis
		return QGis.QGIS_VERSION[0:3] >= "1.6"

	def hasTableColumnEditingSupport(self):
		return False


	def fieldTypes(self):
		return [
			"integer", "bigint", "smallint", # integers
			"real", "double", "float", "numeric", # floats
			"varchar(n)", "character(n)", "text", # strings
			"date", "datetime" # date/time
		]


	def getSchemas(self):
		return None

	def getTables(self, schema=None):
		""" get list of tables """
		tablenames = []
		items = []

		sys_tables = [ "geom_cols_ref_sys", "geometry_columns", "geometry_columns_auth", 
				"views_geometry_columns", "virts_geometry_columns", "spatial_ref_sys", 
				"sqlite_sequence", #"tableprefix_metadata", "tableprefix_rasters", 
				"layer_params","layer_statistics", "layer_sub_classes", "layer_table_layout", 
				"pattern_bitmaps","symbol_bitmaps", "project_defs", "raster_pyramids", 
				"sqlite_stat1", "sqlite_stat2", "spatialite_history" ]

		try:
			vectors = self.getVectorTables(schema)
			for tbl in vectors:
				if tbl[1] in sys_tables:
					continue
				tablenames.append( tbl[1] )
				items.append( tbl )
		except DbError:
			pass

		try:
			rasters = self.getRasterTables(schema)
			for tbl in rasters:
				if tbl[1] in sys_tables:
					continue
				tablenames.append( tbl[1] )
				items.append( tbl )
		except DbError:
			pass

		c = self._get_cursor()

		if self.has_geometry_columns:
			# get the R*Tree tables
			sql = u"SELECT f_table_name, f_geometry_column FROM geometry_columns WHERE spatial_index_enabled = 1"
			self._execute(c, sql)		
			for idx_item in c.fetchall():
				sys_tables.append( 'idx_%s_%s' % idx_item )
				sys_tables.append( 'idx_%s_%s_node' % idx_item )
				sys_tables.append( 'idx_%s_%s_parent' % idx_item )
				sys_tables.append( 'idx_%s_%s_rowid' % idx_item )

			
		sql = u"SELECT name, type = 'view' FROM sqlite_master WHERE type IN ('table', 'view')"
		self._execute(c, sql)

		for tbl in c.fetchall():
			if tablenames.count( tbl[0] ) <= 0:
				item = list(tbl)
				item.insert(0, Table.TableType)
				items.append( item )

		for i, tbl in enumerate(items):
			tbl.insert(3, tbl[1] in sys_tables)

		return sorted( items, cmp=lambda x,y: cmp(x[1], y[1]) )

	def getVectorTables(self, schema=None):
		""" get list of table with a geometry column
			it returns:
				name (table name)
				type = 'view' (is a view?)
				geometry_column:
					f_table_name (the table name in geometry_columns may be in a wrong case, use this to load the layer)
					f_geometry_column
					type 
					coord_dimension 
					srid
		"""
					
		if not self.has_geometry_columns:
			return []

		c = self._get_cursor()
			
		# get geometry info from geometry_columns if exists
		sql = u"""SELECT m.name, m.type = 'view', g.f_table_name, g.f_geometry_column, g.type, g.coord_dimension, g.srid 
						FROM sqlite_master AS m JOIN geometry_columns AS g ON lower(m.name) = lower(g.f_table_name)
						WHERE m.type in ('table', 'view') 
						ORDER BY m.name, g.f_geometry_column"""

		self._execute(c, sql)

		items = []
		for tbl in c.fetchall():
			item = list(tbl)
			item.insert(0, Table.VectorType)
			items.append( item )

		return items


	def getRasterTables(self, schema=None):
		""" get list of table with a geometry column
			it returns:
				name (table name)
				type = 'view' (is a view?)
				geometry_column:
					r.table_name (the prefix table name, use this to load the layer)
					r.geometry_column
					srid
		"""
					
		if not self.has_geometry_columns:
			return []
		if not self.has_raster:
			return []

		c = self._get_cursor()
			
		# get geometry info from geometry_columns if exists
		sql = u"""SELECT r.table_name||'_rasters', m.type = 'view', r.table_name, r.geometry_column, g.srid
						FROM sqlite_master AS m JOIN geometry_columns AS g ON lower(m.name) = lower(g.f_table_name) 
						JOIN layer_params AS r ON REPLACE(m.name, '_metadata', '') = r.table_name
						WHERE m.type in ('table', 'view') AND m.name = r.table_name||'_metadata'
						ORDER BY r.table_name"""

		self._execute(c, sql)

		items = []
		for i, tbl in enumerate(c.fetchall()):
			item = list(tbl)
			item.insert(0, Table.RasterType)
			items.append( item )
			
		return items

	def getTableRowCount(self, table):
		c = self._get_cursor()
		self._execute(c, u"SELECT COUNT(*) FROM %s" % self.quoteId(table) )
		ret = c.fetchone()
		return ret[0] if ret is not None else None

	def getTableFields(self, table):
		""" return list of columns in table """
		c = self._get_cursor()
		sql = u"PRAGMA table_info(%s)" % (self.quoteId(table))
		self._execute(c, sql)
		return c.fetchall()

	def getTableIndexes(self, table):
		""" get info about table's indexes """
		c = self._get_cursor()
		sql = u"PRAGMA index_list(%s)" % (self.quoteId(table))
		self._execute(c, sql)
		indexes = c.fetchall()

		for i, idx in enumerate(indexes):
			num, name, unique = idx
			sql = u"PRAGMA index_info(%s)" % (self.quoteId(name))
			self._execute(c, sql)

			idx = [num, name, unique]
			cols = []
			for seq, cid, cname in c.fetchall():
				cols.append(cid)
			idx.append(cols)
			indexes[i] = idx

		return indexes

	def getTableConstraints(self, table):
		return None

	def getTableTriggers(self, table):
		c = self._get_cursor()
		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT name, sql FROM sqlite_master WHERE lower(tbl_name) = lower(%s) AND type = 'trigger'" % (self.quoteString(tablename))
		self._execute(c, sql)
		return c.fetchall()

	def deleteTableTrigger(self, trigger, table=None):
		""" delete trigger """
		sql = u"DROP TRIGGER %s" % self.quoteId(trigger)
		self._execute_and_commit(sql)


	def getTableExtent(self, table, geom):
		""" find out table extent """
		schema, tablename = self.getSchemaTableName(table)
		c = self._get_cursor()

		if self.isRasterTable(table):
			tablename = QString(tablename).replace('_rasters', '_metadata')
			geom = u'geometry'

		sql = u"""SELECT Min(MbrMinX(%(geom)s)), Min(MbrMinY(%(geom)s)), Max(MbrMaxX(%(geom)s)), Max(MbrMaxY(%(geom)s)) 
						FROM %(table)s """ % { 'geom' : self.quoteId(geom), 'table' : self.quoteId(tablename) }
		self._execute(c, sql)
		return c.fetchone()
	
	def getViewDefinition(self, view):
		""" returns definition of the view """
		schema, tablename = self.getSchemaTableName(view)
		sql = u"SELECT sql FROM sqlite_master WHERE type = 'view' AND name = %s" % self.quoteString(tablename)
		c = self._execute(None, sql)
		ret = c.fetchone()
		return ret[0] if ret is not None else None

	def getSpatialRefInfo(self, srid):
		sql = u"SELECT ref_sys_name FROM spatial_ref_sys WHERE srid = %s" % self.quoteString(srid)
		c = self._execute(None, sql)
		ret = c.fetchone()
		return ret[0] if ret is not None else None

	def isVectorTable(self, table):
		if self.has_geometry_columns:
			schema, tablename = self.getSchemaTableName(table)
			sql = u"SELECT count(*) FROM geometry_columns WHERE f_table_name = %s" % self.quoteString(tablename)
			c = self._execute(None, sql)
			ret = c.fetchone()
			return res != None and ret[0] > 0
		return True

	def isRasterTable(self, table):
		if self.has_geometry_columns and self.has_raster:
			schema, tablename = self.getSchemaTableName(table)
			if not QString(tablename).endsWith( "_rasters" ):
				return False

			sql = u"""SELECT count(*) 
					FROM layer_params AS r JOIN geometry_columns AS g 
						ON r.table_name||'_metadata' = g.f_table_name
					WHERE r.table_name = REPLACE(%s, '_rasters', '')""" % self.quoteString(tablename)
			c = self._execute(None, sql)
			ret = c.fetchone()
			return res != None and ret[0] > 0

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
		""" delete table from the database """
		if self.isRasterTable(table):
			return False

		c = self._get_cursor()
		sql = u"DROP TABLE %s" % self.quoteId(table)
		self._execute(c, sql)
		schema, tablename = self.getSchemaTableName(table)
		sql = u"DELETE FROM geometry_columns WHERE lower(f_table_name) = lower(%s)" % self.quoteString(tablename)
		self._execute(c, sql)
		self._commit()


	def emptyTable(self, table):
		""" delete all rows from table """
		if self.isRasterTable(table):
			return False

		sql = u"DELETE FROM %s" % self.quoteId(table)
		self._execute_and_commit(sql)
		
	def renameTable(self, table, new_table):
		""" rename a table """
		schema, tablename = self.getSchemaTableName(table)
		if new_table == tablename:
			return

		if self.isRasterTable(table):
			return False

		c = self._get_cursor()

		sql = u"ALTER TABLE %s RENAME TO %s" % (self.quoteId(table), self.quoteId(new_table))
		self._execute(c, sql)
		
		# update geometry_columns
		if self.has_geometry_columns:
			sql = u"UPDATE geometry_columns SET f_table_name=%s WHERE f_table_name=%s" % (self.quoteString(new_table), self.quoteString(tablename))
			self._execute(c, sql)

		self._commit()

	def moveTable(self, table, new_table, new_schema=None):
		return self.renameTable(table, new_table)
		
	def createView(self, view, query):
		sql = u"CREATE VIEW %s AS %s" % (self.quoteId(view), query)
		self._execute_and_commit(sql)
	
	def deleteView(self, view):
		sql = u"DROP VIEW %s" % self.quoteId(view)
		self._execute_and_commit(sql)
	
	def renameView(self, view, new_name):
		""" rename view """
		return self.renameTable(view, new_name)


	def runVacuum(self):
		""" run vacuum on the db """
		self._execute_and_commit("VACUUM")


	def addTableColumn(self, table, field_def):
		""" add a column to table """
		sql = u"ALTER TABLE %s ADD %s" % (self.quoteId(table), field_def)
		self._execute_and_commit(sql)
		
	def deleteTableColumn(self, table, column):
		""" delete column from a table """
		if not self.isGeometryColumn(table, column):
			return False	# column editing not supported

		# delete geometry column correctly
		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT DiscardGeometryColumn(%s, %s)" % (self.quoteString(tablename), self.quoteString(column))
		self._execute_and_commit(sql)
		
	def updateTableColumn(self, table, column, new_name, new_data_type=None, new_not_null=None, new_default=None):
		return False	# column editing not supported

	def renameTableColumn(self, table, column, new_name):
		""" rename column in a table """
		return False	# column editing not supported

	def setColumnType(self, table, column, data_type):
		""" change column type """
		return False	# column editing not supported
		
	def setColumnDefault(self, table, column, default):
		""" change column's default value. If default=None drop default value """
		return False	# column editing not supported
		
	def setColumnNull(self, table, column, is_null):
		""" change whether column can contain null values """
		return False	# column editing not supported

	def isGeometryColumn(self, table, column):
		c = self._get_cursor()
		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT count(*) > 0 FROM geometry_columns WHERE lower(f_table_name)=lower(%s) AND lower(f_geometry_column)=lower(%s)" % (self.quoteString(tablename), self.quoteString(column))
		self._execute(c, sql)
		return c.fetchone()[0] == 't'

	def addGeometryColumn(self, table, geom_column='geometry', geom_type='POINT', srid=-1, dim=2):
		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT AddGeometryColumn(%s, %s, %d, %s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column), srid, self.quoteString(geom_type), dim)
		self._execute_and_commit(sql)

	def deleteGeometryColumn(self, table, geom_column):
		return self.deleteTableColumn(table, geom_column)


	def addTableUniqueConstraint(self, table, column):
		""" add a unique constraint to a table """
		return False	# constraints not supported

	def deleteTableConstraint(self, table, constraint):
		""" delete constraint in a table """
		return False	# constraints not supported


	def addTablePrimaryKey(self, table, column):
		""" add a primery key (with one column) to a table """
		sql = u"ALTER TABLE %s ADD PRIMARY KEY (%s)" % (self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)


	def createTableIndex(self, table, name, column, unique=False):
		""" create index on one column using default options """
		unique_str = u"UNIQUE" if unique else ""
		sql = u"CREATE %s INDEX %s ON %s (%s)" % (unique_str, self.quoteId(name), self.quoteId(table), self.quoteId(column))
		self._execute_and_commit(sql)
		
	def deleteTableIndex(self, table, name):
		schema, tablename = self.getSchemaTableName(table)
		sql = u"DROP INDEX %s" % self.quoteId( (schema, name) )
		self._execute_and_commit(sql)

	def createSpatialIndex(self, table, geom_column='geometry'):
		if self.isRasterTable( table ):
			return False

		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT CreateSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
		self._execute_and_commit(sql)
			
	def deleteSpatialIndex(self, table, geom_column='geometry'):
		if self.isRasterTable( table ):
			return False

		schema, tablename = self.getSchemaTableName(table)
		try:
			sql = u"SELECT DiscardSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
			self._execute_and_commit(sql)
		except DbError:
			sql = u"SELECT DeleteSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
			self._execute_and_commit(sql)
			# delete the index table
			idx_table_name = u"idx_%s_%s" % (tablename, geom_column)
			self.deleteTable(idx_table_name)

	def hasSpatialIndex(self, table, geom_column='geometry'):
		if not self.has_geometry_columns or self.isRasterTable( table ):
			return False
		c = self._get_cursor()
		schema, tablename = self.getSchemaTableName(table)
		sql = u"SELECT spatial_index_enabled FROM geometry_columns WHERE f_table_name = %s AND f_geometry_column = %s" % (self.quoteString(tablename), self.quoteString(geom_column))
		self._execute(c, sql)
		row = c.fetchone()
		return row != None and row[0] == 1


	def execution_error_types(self):
		return sqlite.Error, sqlite.ProgrammingError

	def connection_error_types(self):
		return sqlite.InterfaceError, sqlite.OperationalError

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
		return getSqlDictionary()

