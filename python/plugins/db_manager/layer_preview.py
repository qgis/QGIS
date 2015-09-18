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

from PyQt4.QtCore import Qt, QSettings, QTimer, SIGNAL
from PyQt4.QtGui import QColor, QApplication, QCursor

from qgis.gui import QgsMapCanvas, QgsMapCanvasLayer, QgsMessageBar
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry

from .db_plugins.plugin import Table


class LayerPreview(QgsMapCanvas):

    def __init__(self, parent=None):
        QgsMapCanvas.__init__(self, parent)
        self.parent = parent
        self.setCanvasColor(QColor(255, 255, 255))

        self.item = None
        self.dirty = False
        self.currentLayer = None

        # reuse settings from QGIS
        settings = QSettings()
        self.enableAntiAliasing(settings.value("/qgis/enable_anti_aliasing", False, type=bool))
        action = settings.value("/qgis/wheel_action", 0, type=float)
        zoomFactor = settings.value("/qgis/zoom_factor", 2, type=float)
        self.setWheelAction(QgsMapCanvas.WheelAction(action), zoomFactor)

    def refresh(self):
        self.setDirty(True)
        self.loadPreview(self.item)

    def loadPreview(self, item, force=False):
        if item == self.item and not self.dirty:
            return
        self._clear()
        if item is None:
            return

        if isinstance(item, Table) and item.type in [Table.VectorType, Table.RasterType]:
            # update the preview, but first let the manager chance to show the canvas
            runPrev = lambda: self._loadTablePreview(item)
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
            ## skip exception on RuntimeError fixes #6892
            try:
                self.disconnect(self.item, SIGNAL('aboutToChange'), self.setDirty)
            except RuntimeError:
                pass

        self.item = None
        self.dirty = False
        self._loadTablePreview(None)

    def _loadTablePreview(self, table, limit=False):
        """ if has geometry column load to map canvas """
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        self.setRenderFlag(False)
        vl = None

        if table and table.geomType:
            # limit the query result if required
            if limit and table.rowCount > 1000:
                uniqueField = table.getValidQGisUniqueFields(True)
                if uniqueField is None:
                    self.parent.tabs.setCurrentWidget(self.parent.info)
                    self.parent.infoBar.pushMessage(
                        QApplication.translate("DBManagerPlugin", "Unable to find a valid unique field"),
                        QgsMessageBar.WARNING, self.parent.iface.messageTimeout())
                    return

                uri = table.database().uri()
                uri.setDataSource("", u"(SELECT * FROM %s LIMIT 1000)" % table.quotedName(), table.geomColumn, "",
                                  uniqueField.name)
                provider = table.database().dbplugin().providerName()
                vl = QgsVectorLayer(uri.uri(), table.name, provider)
            else:
                vl = table.toMapLayer()

            if not vl.isValid():
                vl.deleteLater()
                vl = None

        # remove old layer (if any) and set new
        if self.currentLayer:
            QgsMapLayerRegistry.instance().removeMapLayers([self.currentLayer.id()])

        if vl:
            self.setLayerSet([QgsMapCanvasLayer(vl)])
            QgsMapLayerRegistry.instance().addMapLayers([vl], False)
            self.zoomToFullExtent()
        else:
            self.setLayerSet([])

        self.currentLayer = vl

        self.setRenderFlag(True)
        QApplication.restoreOverrideCursor()
