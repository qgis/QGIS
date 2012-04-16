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

from qgis.gui import QgsMapCanvas, QgsMapCanvasLayer
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry

from .db_plugins.plugin import DbError, Table

class LayerPreview(QgsMapCanvas):
	def __init__(self, parent=None):
		QgsMapCanvas.__init__(self, parent)
		self.setCanvasColor(QColor(255,255,255))

		self.item = None
		self.dirty = False

		# reuse settings from QGIS
		settings = QSettings()
		self.enableAntiAliasing( settings.value( "/qgis/enable_anti_aliasing", QVariant(False) ).toBool() )
		self.useImageToRender( settings.value( "/qgis/use_qimage_to_render", QVariant(False) ).toBool() )
		action = settings.value( "/qgis/wheel_action", QVariant(0) ).toInt()[0]
		zoomFactor = settings.value( "/qgis/zoom_factor", QVariant(2) ).toDouble()[0]
		self.setWheelAction( QgsMapCanvas.WheelAction(action), zoomFactor )

		self._clear()


	def refresh(self):
		self.setDirty(True)
		self.loadPreview( self.item )		

	def loadPreview(self, item, force=False):
		if item == self.item and not self.dirty: 
			return
		self._clear()
		if item is None:
			return

		if isinstance(item, Table) and item.type in [Table.VectorType, Table.RasterType]:
			# update the preview, but first let the manager chance to show the canvas
			runPrev = lambda: self._loadTablePreview( item )
			QTimer.singleShot(50, runPrev)
		else:
			return

		self.item = item
		self.connect(self.item, SIGNAL('aboutToChange'), self.setDirty)

	def setDirty(self, val=True):
		self.dirty = val

	def _clear(self):
		""" remove any layers from preview canvas """
		if self.item is not None:
			self.disconnect(self.item, SIGNAL('aboutToChange'), self.setDirty)
		self.item = None
		self.dirty = False

		self.currentLayerId = None
		self.setLayerSet( [] )

	def _loadTablePreview(self, table, limit=False):
		""" if has geometry column load to map canvas """
		QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
		self.setRenderFlag(False)
		newLayerId = None

		if table.geomType:
			# limit the query result if required
			if limit and table.rowCount > 1000:
				uniqueField = table.getValidQGisUniqueFields(True)
				if uniqueField == None:
					QMessageBox.warning(self, "Sorry", "Unable to find a valid unique field")
					return

				uri = table.database().uri()
				uri.setDataSource("", u"(SELECT * FROM %s LIMIT 1000)" % table.quotedName(), table.geomColumn, "", uniqueField.name)
				provider = table.database().dbplugin().providerName()
				vl = QgsVectorLayer(uri.uri(), table.name, provider)
			else:
				vl = table.toMapLayer()

			if not vl.isValid():
				self.setLayerSet( [] )
			else:
				newLayerId = vl.id() if hasattr(vl, 'id') else vl.getLayerID()
				self.setLayerSet( [ QgsMapCanvasLayer(vl) ] )
				QgsMapLayerRegistry.instance().addMapLayer(vl, False)
				self.zoomToFullExtent()

		# remove old layer (if any) and set new
		if self.currentLayerId:
			QgsMapLayerRegistry.instance().removeMapLayer(self.currentLayerId, False)
		self.currentLayerId = newLayerId

		self.setRenderFlag(True)
		QApplication.restoreOverrideCursor()

