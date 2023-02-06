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

from qgis.PyQt.QtCore import Qt, QTimer
from qgis.PyQt.QtGui import QColor, QCursor
from qgis.PyQt.QtWidgets import QApplication

from qgis.gui import QgsMapCanvas, QgsMessageBar
from qgis.core import Qgis, QgsVectorLayer, QgsProject, QgsSettings
from qgis.utils import OverrideCursor

from .db_plugins.plugin import Table


class LayerPreview(QgsMapCanvas):

    def __init__(self, parent=None):
        super(LayerPreview, self).__init__(parent)
        self.parent = parent
        self.setCanvasColor(QColor(255, 255, 255))

        self.item = None
        self.dirty = False
        self.current_layer = None

        # reuse settings from QGIS
        settings = QgsSettings()
        self.enableAntiAliasing(settings.value("/qgis/enable_anti_aliasing", False, type=bool))
        zoomFactor = settings.value("/qgis/zoom_factor", 2, type=float)
        self.setWheelFactor(zoomFactor)

    def refresh(self):
        self.setDirty(True)
        self.loadPreview(self.item)

    def loadPreview(self, item):
        if item == self.item and not self.dirty:
            return

        if item is None:
            return

        self._clear()

        if isinstance(item, Table) and item.type in [Table.VectorType, Table.RasterType]:
            # update the preview, but first let the manager chance to show the canvas
            def runPrev():
                return self._loadTablePreview(item)

            QTimer.singleShot(50, runPrev)
        else:
            return

        self.item = item
        self.item.aboutToChange.connect(self.setDirty)

    def setDirty(self, val=True):
        self.dirty = val

    def _clear(self):
        """ remove any layers from preview canvas """
        if self.item is not None:
            # skip exception on RuntimeError fixes #6892
            try:
                self.item.aboutToChange.disconnect(self.setDirty)
            except RuntimeError:
                pass

        self.item = None
        self.dirty = False
        self._loadTablePreview(None)

    def _loadTablePreview(self, table, limit=False):
        """ if has geometry column load to map canvas """
        with OverrideCursor(Qt.WaitCursor):
            self.freeze()
            vl = None

            if table and table.geomType:
                # limit the query result if required
                if limit and table.rowCount > 1000:
                    uniqueField = table.getValidQgisUniqueFields(True)
                    if uniqueField is None:
                        self.parent.tabs.setCurrentWidget(self.parent.info)
                        self.parent.infoBar.pushMessage(
                            QApplication.translate("DBManagerPlugin", "Unable to find a valid unique field"),
                            Qgis.Warning, self.parent.iface.messageTimeout())
                        return

                    uri = table.database().uri()
                    uri.setDataSource("", "(SELECT * FROM %s LIMIT 1000)" % table.quotedName(), table.geomColumn, "",
                                      uniqueField.name)
                    provider = table.database().dbplugin().providerName()
                    vl = QgsVectorLayer(uri.uri(False), table.name, provider)
                else:
                    vl = table.toMapLayer()

                if vl and not vl.isValid():
                    vl.deleteLater()
                    vl = None

            if vl and vl.isValid():
                self.current_layer = vl
                self.setLayers([self.current_layer])
                self.zoomToFullExtent()
            else:
                self.setLayers([])
                self.current_layer = None

            self.freeze(False)
            super().refresh()
