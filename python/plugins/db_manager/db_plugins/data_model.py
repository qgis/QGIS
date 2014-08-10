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

from .plugin import DbError

class BaseTableModel(QAbstractTableModel):
	def __init__(self, header=None, data=None, parent=None):
		QAbstractTableModel.__init__(self, parent)
		self._header = header if header else []
		self.resdata = data if data else []

	def headerToString(self, sep=u"\t"):
		header = self._header
		return sep.join(header)

	def rowToString(self, row, sep=u"\t"):
		text = u""
		for col in range(self.columnCount()):
			text += u"%s" % self.getData(row, col) + sep
		return text[:-1]

	def getData(self, row, col):
		return self.resdata[row][col]

	def columnNames(self):
		return list(self._header)

	def rowCount(self, parent=None):
		return len(self.resdata)

	def columnCount(self, parent=None):
		return len(self._header)

	def data(self, index, role):
		if role != Qt.DisplayRole and role != Qt.FontRole:
			return None

		val = self.getData(index.row(), index.column())

		if role == Qt.FontRole:	# draw NULL in italic
			if val != None:
				return None
			f = QFont()
			f.setItalic(True)
			return f

		if val == None:
			return "NULL"
		elif isinstance(val, buffer):
			# hide binary data
			return None
		elif isinstance(val, (str, unicode)) and len(val) > 300:
			# too much data to display, elide the string
			return u"%s..." % val[:300]
		return unicode(val)	# convert to string

	def headerData(self, section, orientation, role):
		if role != Qt.DisplayRole:
			return None

		if orientation == Qt.Vertical:
			# header for a row
			return section+1
		else:
			# header for a column
			return self._header[section]


class TableDataModel(BaseTableModel):
	def __init__(self, table, parent=None):
		self.db = table.database().connector
		self.table = table

		fieldNames = map(lambda x: x.name, table.fields())
		BaseTableModel.__init__(self, fieldNames, None, parent)

		# get table fields
		self.fields = []
		for fld in table.fields():
			self.fields.append( self._sanitizeTableField(fld) )

		self.fetchedCount = 201
		self.fetchedFrom = -self.fetchedCount-1		# so the first call to getData will exec fetchMoreData(0)

	def _sanitizeTableField(self, field):
		""" quote column names to avoid some problems (e.g. columns with upper case) """
		return self.db.quoteId(field)

	def getData(self, row, col):
		if row < self.fetchedFrom or row >= self.fetchedFrom + self.fetchedCount:
			margin = self.fetchedCount/2
			start = self.rowCount() - margin if row + margin >= self.rowCount() else row - margin
			if start < 0: start = 0
			self.fetchMoreData(start)
		return self.resdata[row-self.fetchedFrom][col]

	def fetchMoreData(self, row_start):
		pass

	def rowCount(self, index=None):
		# case for tables with no columns ... any reason to use them? :-)
		return self.table.rowCount if self.table.rowCount != None and self.columnCount(index) > 0 else 0


class SqlResultModel(BaseTableModel):
	def __init__(self, db, sql, parent=None):
		self.db = db.connector

		t = QTime()
		t.start()
		c = self.db._execute(None, unicode(sql))
		self._secs = t.elapsed() / 1000.0
		del t

		self._affectedRows = 0
		data = []
		header = self.db._get_cursor_columns(c)
		if header == None:
			header = []

		try:
			if len(header) > 0:
				data = self.db._fetchall(c)
			self._affectedRows = c.rowcount
		except DbError:
			# nothing to fetch!
			data = []
			header = []

		BaseTableModel.__init__(self, header, data, parent)

		# commit before closing the cursor to make sure that the changes are stored
		self.db._commit()
		c.close()
		del c

	def secs(self):
		return self._secs

	def affectedRows(self):
		return self._affectedRows



class SimpleTableModel(QStandardItemModel):
	def __init__(self, header, editable=False, parent=None):
		self.header = header
		self.editable = editable
		QStandardItemModel.__init__(self, 0, len(self.header), parent)

	def rowFromData(self, data):
		row = []
		for c in data:
			item = QStandardItem(unicode(c))
			item.setFlags( (item.flags() | Qt.ItemIsEditable) if self.editable else (item.flags() & ~Qt.ItemIsEditable) )
			row.append( item )
		return row

	def headerData(self, section, orientation, role):
		if orientation == Qt.Horizontal and role == Qt.DisplayRole:
			return self.header[section]
		return None

	def _getNewObject(self):
		pass

	def getObject(self, row):
		return self._getNewObject()

	def getObjectIter(self):
		for row in range(self.rowCount()):
			yield self.getObject(row)



class TableFieldsModel(SimpleTableModel):
	def __init__(self, parent, editable=False):
		SimpleTableModel.__init__(self, ['Name', 'Type', 'Null', 'Default'], editable, parent)

	def headerData(self, section, orientation, role):
		if orientation == Qt.Vertical and role == Qt.DisplayRole:
			return section+1
		return SimpleTableModel.headerData(self, section, orientation, role)

	def flags(self, index):
		flags = SimpleTableModel.flags(self, index)
		if index.column() == 2:  # set Null column as checkable
			flags &= ~Qt.ItemIsEditable
			flags |= Qt.ItemIsUserCheckable
		return flags

	def append(self, fld):
		data = [fld.name, fld.type2String(), not fld.notNull, fld.default2String()]
		self.appendRow( self.rowFromData(data) )
		row = self.rowCount()-1
		self.setData(self.index(row, 0), fld, Qt.UserRole)
		self.setData(self.index(row, 1), fld.primaryKey, Qt.UserRole)

	def _getNewObject(self):
		from .plugin import TableField
		return TableField(None)

	def getObject(self, row):
		val = self.data(self.index(row, 0), Qt.UserRole)
		fld = val if val != None else self._getNewObject()
		fld.name = self.data(self.index(row, 0)) or ""

		typestr = self.data(self.index(row, 1)) or ""
		regex = QRegExp( "([^\(]+)\(([^\)]+)\)" )
		startpos = regex.indexIn( typestr )
		if startpos >= 0:
			fld.dataType = regex.cap(1).strip()
			fld.modifier = regex.cap(2).strip()
		else:
			fld.modifier = None
			fld.dataType = typestr

		fld.notNull = self.data(self.index(row, 2), Qt.CheckStateRole) == Qt.Unchecked
		fld.primaryKey = self.data(self.index(row, 1), Qt.UserRole)
		return fld

	def getFields(self):
		flds = []
		for fld in self.getObjectIter():
			flds.append( fld )
		return flds


class TableConstraintsModel(SimpleTableModel):
	def __init__(self, parent, editable=False):
		SimpleTableModel.__init__(self, [QApplication.translate("DBManagerPlugin", 'Name'), QApplication.translate("DBManagerPlugin", 'Type'), QApplication.translate("DBManagerPlugin", 'Column(s)')], editable, parent)

	def append(self, constr):
		field_names = map( lambda (k,v): unicode(v.name), constr.fields().iteritems() )
		data = [constr.name, constr.type2String(), u", ".join(field_names)]
		self.appendRow( self.rowFromData(data) )
		row = self.rowCount()-1
		self.setData(self.index(row, 0), constr, Qt.UserRole)
		self.setData(self.index(row, 1), constr.type, Qt.UserRole)
		self.setData(self.index(row, 2), constr.columns, Qt.UserRole)

	def _getNewObject(self):
		from .plugin import TableConstraint
		return TableConstraint(None)

	def getObject(self, row):
		constr = self.data(self.index(row, 0), Qt.UserRole)
		if not constr:
			constr = self._getNewObject()
		constr.name = self.data(self.index(row, 0)) or ""
		constr.type = self.data(self.index(row, 1), Qt.UserRole)
		constr.columns = self.data(self.index(row, 2), Qt.UserRole)
		return constr

	def getConstraints(self):
		constrs = []
		for constr in self.getObjectIter():
			constrs.append( constr )
		return constrs


class TableIndexesModel(SimpleTableModel):
	def __init__(self, parent, editable=False):
		SimpleTableModel.__init__(self, [QApplication.translate("DBManagerPlugin", 'Name'), QApplication.translate("DBManagerPlugin", 'Column(s)')], editable, parent)

	def append(self, idx):
		field_names = map( lambda (k,v): unicode(v.name), idx.fields().iteritems() )
		data = [idx.name, u", ".join(field_names)]
		self.appendRow( self.rowFromData(data) )
		row = self.rowCount()-1
		self.setData(self.index(row, 0), idx, Qt.UserRole)
		self.setData(self.index(row, 1), idx.columns, Qt.UserRole)

	def _getNewObject(self):
		from .plugin import TableIndex
		return TableIndex(None)

	def getObject(self, row):
		idx = self.data(self.index(row, 0), Qt.UserRole)
		if not idx:
			idx = self._getNewObject()
		idx.name = self.data(self.index(row, 0))
		idx.columns = self.data(self.index(row, 1), Qt.UserRole)
		return idx

	def getIndexes(self):
		idxs = []
		for idx in self.getObjectIter():
			idxs.append( idx )
		return idxs
