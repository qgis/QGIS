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

from .db_plugins.plugin import DbError, Table
from .dlg_db_error import DlgDbError

class TableViewer(QTableView):
	def __init__(self, parent=None):
		QTableView.__init__(self, parent)
		self.setSelectionBehavior( QAbstractItemView.SelectRows )
		self.setSelectionMode( QAbstractItemView.ExtendedSelection )

		self.item = None
		self.dirty = False

		# allow to copy results
		copyAction = QAction("copy", self)
		self.addAction( copyAction )
		copyAction.setShortcuts(QKeySequence.Copy)
		QObject.connect(copyAction, SIGNAL("triggered()"), self.copySelectedResults)

		self._clear()

	def refresh(self):
		self.dirty = True
		self.loadData( self.item )

	def loadData(self, item ):
		if item == self.item and not self.dirty:
			return
		self._clear()
		if item is None:
			return

		if isinstance(item, Table):
			self._loadTableData( item )
		else:
			return

		self.item = item
		self.connect(self.item, SIGNAL('aboutToChange'), self.setDirty)

	def setDirty(self, val=True):
		self.dirty = val

	def _clear(self):
		if self.item is not None:
			try:
				self.disconnect(self.item, SIGNAL('aboutToChange'), self.setDirty)
			except:
				# do not raise any error if self.item was deleted
				pass

		self.item = None
		self.dirty = False

		# delete the old model
		model = self.model()
		self.setModel(None)
		if model: model.deleteLater()

	def _loadTableData(self, table):
		QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
		try:
			# set the new model
			self.setModel( table.tableDataModel(self) )

		except DbError, e:
			DlgDbError.showError(e, self)
			return

		else:
			self.update()

		finally:
			QApplication.restoreOverrideCursor()


	def copySelectedResults(self):
		if len(self.selectedIndexes()) <= 0:
			return
		model = self.model()

		# convert to string using tab as separator
		text = model.headerToString( "\t" )
		for idx in self.selectionModel().selectedRows():
			text += "\n" + model.rowToString( idx.row(), "\t" )

		QApplication.clipboard().setText( text, QClipboard.Selection )
		QApplication.clipboard().setText( text, QClipboard.Clipboard )


