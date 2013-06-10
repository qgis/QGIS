# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
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

from qgis.core import QgsDataSourceURI

from .plugin import BaseError, DbError, ConnectionError

class DBConnector:
	def __init__(self, uri):
		self.connection = None
		self._uri = uri

	def __del__(self):
		pass	#print "DBConnector.__del__", self._uri.connectionInfo()
		if self.connection != None:
			self.connection.close()
		self.connection = None


	def uri(self):
		return QgsDataSourceURI( self._uri.uri() )

	def publicUri(self):
		publicUri = QgsDataSourceURI.removePassword( self._uri.uri() )
		return QgsDataSourceURI( publicUri )


	def hasSpatialSupport(self):
		return False

	def hasRasterSupport(self):
		return False

	def hasCustomQuerySupport(self):
		return False

	def hasTableColumnEditingSupport(self):
		return False


	def execution_error_types(self):
		raise Exception("DBConnector.execution_error_types() is an abstract method")

	def connection_error_types(self):
		raise Exception("DBConnector.connection_error_types() is an abstract method")

	def error_types(self):
		return self.connection_error_types() + self.execution_error_types()

	def _execute(self, cursor, sql):
		if cursor == None:
			cursor = self._get_cursor()
		try:
			cursor.execute(unicode(sql))

		except self.connection_error_types() as e:
			raise ConnectionError(e)

		except self.execution_error_types() as e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e, sql)

		return cursor

	def _execute_and_commit(self, sql):
		""" tries to execute and commit some action, on error it rolls back the change """
		self._execute(None, sql)
		self._commit()

	def _get_cursor(self, name=None):
		try:
			if name != None:
				name = unicode(name).encode('ascii', 'replace').replace( '?', "_" )
				self._last_cursor_named_id = 0 if not hasattr(self, '_last_cursor_named_id') else self._last_cursor_named_id + 1
				return self.connection.cursor( "%s_%d" % (name, self._last_cursor_named_id) )

			return self.connection.cursor()

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		except self.execution_error_types(), e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e)

	def _close_cursor(self, c):
		try:
			if c and not c.closed:
				c.close()

		except self.error_types(), e:
			pass

		return


	def _fetchall(self, c):
		try:
			return c.fetchall()

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		except self.execution_error_types(), e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e)

	def _fetchone(self, c):
		try:
			return c.fetchone()

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		except self.execution_error_types(), e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e)


	def _commit(self):
		try:
			self.connection.commit()

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		except self.execution_error_types(), e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e)


	def _rollback(self):
		try:
			self.connection.rollback()

		except self.connection_error_types(), e:
			raise ConnectionError(e)

		except self.execution_error_types(), e:
			# do the rollback to avoid a "current transaction aborted, commands ignored" errors
			self._rollback()
			raise DbError(e)


	def _get_cursor_columns(self, c):
		try:
			if c.description:
				return map(lambda x: x[0], c.description)

		except self.connection_error_types() + self.execution_error_types(), e:
			return []


	@classmethod
	def quoteId(self, identifier):
		if hasattr(identifier, '__iter__'):
			ids = list()
			for i in identifier:
				if i == None:
					continue
				ids.append( self.quoteId(i) )
			return u'.'.join( ids )

		identifier = unicode(identifier) if identifier != None else unicode() # make sure it's python unicode string
		return u'"%s"' % identifier.replace('"', '""')

	@classmethod
	def quoteString(self, txt):
		""" make the string safe - replace ' with '' """
		if hasattr(txt, '__iter__'):
			txts = list()
			for i in txt:
				if i == None:
					continue
				txts.append( self.quoteString(i) )
			return u'.'.join( txts )

		txt = unicode(txt) if txt != None else unicode() # make sure it's python unicode string
		return u"'%s'" % txt.replace("'", "''")

	@classmethod
	def getSchemaTableName(self, table):
		if not hasattr(table, '__iter__'):
			return (None, table)
		elif len(table) < 2:
			return (None, table[0])
		else:
			return (table[0], table[1])

	@classmethod
	def getSqlDictionary(self):
		""" return a generic SQL dictionary """
		try:
			from ..sql_dictionary import getSqlDictionary
			return getSqlDictionary()
		except ImportError:
			return []

