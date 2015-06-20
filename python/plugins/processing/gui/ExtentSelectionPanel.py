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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4 import uic
from PyQt4.QtGui import QMenu, QAction, QCursor, QInputDialog

from qgis.core import QgsRasterLayer, QgsVectorLayer
from qgis.utils import iface

from processing.gui.RectangleMapTool import RectangleMapTool
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterMultipleInput
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class ExtentSelectionPanel(BASE, WIDGET):

    def __init__(self, dialog, alg, default):
        super(ExtentSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.dialog = dialog
        self.params = alg.parameters
        if self.canUseAutoExtent():
            if hasattr(self.leText, 'setPlaceholderText'):
                self.leText.setPlaceholderText(
                    self.tr('[Leave blank to use min covering extent]'))

        self.btnSelect.clicked.connect(self.selectExtent)

        canvas = iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()
        self.tool = RectangleMapTool(canvas)
        self.tool.rectangleCreated.connect(self.updateExtent)

    def canUseAutoExtent(self):
        for param in self.params:
            if isinstance(param, (ParameterRaster, ParameterVector)):
                return True
            if isinstance(param, ParameterMultipleInput):
                return True

        return False

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

        if self.canUseAutoExtent():
            useMincoveringExtentAction = QAction(
                self.tr('Use min covering extent from input layers'),
                self.btnSelect)
            useMincoveringExtentAction.triggered.connect(
                self.useMinCoveringExtent)
            popupmenu.addAction(useMincoveringExtentAction)

        popupmenu.exec_(QCursor.pos())

    def useMinCoveringExtent(self):
        self.leText.setText('')

    def getMinCoveringExtent(self):
        first = True
        found = False
        for param in self.params:
            if param.value:
                if isinstance(param, (ParameterRaster, ParameterVector)):
                    if isinstance(param.value, (QgsRasterLayer,
                                  QgsVectorLayer)):
                        layer = param.value
                    else:
                        layer = dataobjects.getObjectFromUri(param.value)
                    if layer:
                        found = True
                        self.addToRegion(layer, first)
                        first = False
                elif isinstance(param, ParameterMultipleInput):
                    layers = param.value.split(';')
                    for layername in layers:
                        layer = dataobjects.getObjectFromUri(layername, first)
                        if layer:
                            found = True
                            self.addToRegion(layer, first)
                            first = False
        if found:
            return '{},{},{},{}'.format(
                self.xmin, self.xmax, self.ymin, self.ymax)
        else:
            return None

    def useNewAlg(self, alg):
        self.params = alg.parameters

    def addToRegion(self, layer, first):
        if first:
            self.xmin = layer.extent().xMinimum()
            self.xmax = layer.extent().xMaximum()
            self.ymin = layer.extent().yMinimum()
            self.ymax = layer.extent().yMaximum()
        else:
            self.xmin = min(self.xmin, layer.extent().xMinimum())
            self.xmax = max(self.xmax, layer.extent().xMaximum())
            self.ymin = min(self.ymin, layer.extent().yMinimum())
            self.ymax = max(self.ymax, layer.extent().yMaximum())

    def useLayerExtent(self):
        CANVAS_KEY = 'Use canvas extent'
        extentsDict = {}
        extentsDict[CANVAS_KEY] = iface.mapCanvas().extent()
        extents = [CANVAS_KEY]
        layers = dataobjects.getAllLayers()
        for layer in layers:
            extents.append(layer.name())
            extentsDict[layer.name()] = layer.extent()
        (item, ok) = QInputDialog.getItem(self, self.tr('Select extent'),
                self.tr('Use extent from'), extents, False)
        if ok:
            self.setValueFromRect(extentsDict[item])

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
        if str(self.leText.text()).strip() != '':
            return unicode(self.leText.text())
        else:
            return self.getMinCoveringExtent()

    def setExtentFromString(self, s):
        self.leText.setText(s)
