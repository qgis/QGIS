# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtentSelectionPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QMenu, QAction, QInputDialog
from qgis.PyQt.QtGui import QCursor

from qgis.gui import QgsMessageBar
from qgis.core import QgsRasterLayer, QgsVectorLayer
from qgis.utils import iface

from processing.gui.RectangleMapTool import RectangleMapTool
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterMultipleInput
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class ExtentSelectionPanel(BASE, WIDGET):

    def __init__(self, dialog, alg, default=None):
        super(ExtentSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.dialog = dialog
        self.alg = alg
        if alg.canUseAutoExtent():
            if hasattr(self.leText, 'setPlaceholderText'):
                self.leText.setPlaceholderText(
                    self.tr('[Leave blank to use min covering extent]'))

        self.btnSelect.clicked.connect(self.selectExtent)

        canvas = iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()
        self.tool = RectangleMapTool(canvas)
        self.tool.rectangleCreated.connect(self.updateExtent)

        if default:
            tokens = str(default).split(',')
            if len(tokens) == 4:
                try:
                    float(tokens[0])
                    float(tokens[1])
                    float(tokens[2])
                    float(tokens[3])
                    self.leText.setText(str(default))
                except:
                    pass

    def selectExtent(self):
        popupmenu = QMenu()
        useLayerExtentAction = QAction(
            self.tr('Use layer/canvas extent'), self.btnSelect)
        selectOnCanvasAction = QAction(
            self.tr('Select extent on canvas'), self.btnSelect)

        popupmenu.addAction(useLayerExtentAction)
        popupmenu.addAction(selectOnCanvasAction)

        selectOnCanvasAction.triggered.connect(self.selectOnCanvas)
        useLayerExtentAction.triggered.connect(self.useLayerExtent)

        if self.alg.canUseAutoExtent():
            useMincoveringExtentAction = QAction(
                self.tr('Use min covering extent from input layers'),
                self.btnSelect)
            useMincoveringExtentAction.triggered.connect(
                self.useMinCoveringExtent)
            popupmenu.addAction(useMincoveringExtentAction)

        popupmenu.exec_(QCursor.pos())

    def useMinCoveringExtent(self):
        self.leText.setText('')

    def useLayerExtent(self):
        CANVAS_KEY = 'Use canvas extent'
        extentsDict = {}
        extentsDict[CANVAS_KEY] = {"extent": iface.mapCanvas().extent(),
                                   "authid": iface.mapCanvas().mapSettings().destinationCrs().authid()}
        extents = [CANVAS_KEY]
        layers = dataobjects.getAllLayers()
        for layer in layers:
            authid = layer.crs().authid()
            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF) \
                    and authid is not None:
                layerName = u'{} [{}]'.format(layer.name(), authid)
            else:
                layerName = layer.name()
            extents.append(layerName)
            extentsDict[layerName] = {"extent": layer.extent(), "authid": authid}
        (item, ok) = QInputDialog.getItem(self, self.tr('Select extent'),
                                          self.tr('Use extent from'), extents, False)
        if ok:
            self.setValueFromRect(extentsDict[item]["extent"])
            if extentsDict[item]["authid"] != iface.mapCanvas().mapSettings().destinationCrs().authid():
                iface.messageBar().pushMessage(self.tr("Warning"),
                                               self.tr("The projection of the chosen layer is not the same as canvas projection! The selected extent might not be what was intended."),
                                               QgsMessageBar.WARNING, 8)

    def selectOnCanvas(self):
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.tool)
        self.dialog.showMinimized()

    def updateExtent(self):
        r = self.tool.rectangle()
        self.setValueFromRect(r)

    def setValueFromRect(self, r):
        s = '{},{},{},{}'.format(
            r.xMinimum(), r.xMaximum(), r.yMinimum(), r.yMaximum())

        self.leText.setText(s)
        self.tool.reset()
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        self.dialog.showNormal()
        self.dialog.raise_()
        self.dialog.activateWindow()

    def getValue(self):
        if str(self.leText.text()).strip() == '':
            return None
        return str(self.leText.text())

    def setExtentFromString(self, s):
        self.leText.setText(s)
