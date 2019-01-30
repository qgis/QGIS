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
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import (
    QMenu,
    QAction,
    QDialog,
    QVBoxLayout,
    QDialogButtonBox,
    QLabel
)
from qgis.PyQt.QtGui import QCursor
from qgis.PyQt.QtCore import QCoreApplication, pyqtSignal

from qgis.gui import QgsMapLayerComboBox
from qgis.utils import iface
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameters,
                       QgsProject,
                       QgsReferencedRectangle,
                       QgsMapLayerProxyModel)
from processing.gui.RectangleMapTool import RectangleMapTool
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.dataobjects import createContext

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class LayerSelectionDialog(QDialog):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle(self.tr('Select Extent'))

        vl = QVBoxLayout()
        vl.addWidget(QLabel(self.tr('Use extent from')))
        self.combo = QgsMapLayerComboBox()
        self.combo.setFilters(
            QgsMapLayerProxyModel.HasGeometry | QgsMapLayerProxyModel.RasterLayer | QgsMapLayerProxyModel.MeshLayer)
        self.combo.setShowCrs(ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF))
        vl.addWidget(self.combo)

        self.button_box = QDialogButtonBox()
        self.button_box.setStandardButtons(QDialogButtonBox.Cancel | QDialogButtonBox.Ok)
        self.button_box.accepted.connect(self.accept)
        self.button_box.rejected.connect(self.reject)

        vl.addWidget(self.button_box)
        self.setLayout(vl)

    def selected_layer(self):
        return self.combo.currentLayer()


class ExtentSelectionPanel(BASE, WIDGET):

    hasChanged = pyqtSignal()

    def __init__(self, dialog, param):
        super(ExtentSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.leText.textChanged.connect(lambda: self.hasChanged.emit())

        self.dialog = dialog
        self.param = param
        self.crs = QgsProject.instance().crs()

        if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
            if hasattr(self.leText, 'setPlaceholderText'):
                self.leText.setPlaceholderText(
                    self.tr('[Leave blank to use min covering extent]'))

        self.btnSelect.clicked.connect(self.selectExtent)

        if iface is not None:
            canvas = iface.mapCanvas()
            self.prevMapTool = canvas.mapTool()
            self.tool = RectangleMapTool(canvas)
            self.tool.rectangleCreated.connect(self.updateExtent)
        else:
            self.prevMapTool = None
            self.tool = None

        if param.defaultValue() is not None:
            context = createContext()
            rect = QgsProcessingParameters.parameterAsExtent(param, {param.name(): param.defaultValue()}, context)
            crs = QgsProcessingParameters.parameterAsExtentCrs(param, {param.name(): param.defaultValue()}, context)
            if not rect.isNull():
                try:
                    s = '{},{},{},{}'.format(
                        rect.xMinimum(), rect.xMaximum(), rect.yMinimum(), rect.yMaximum())
                    if crs.isValid():
                        s += ' [' + crs.authid() + ']'
                        self.crs = crs
                    self.leText.setText(s)
                except:
                    pass

    def selectExtent(self):
        popupmenu = QMenu()
        useCanvasExtentAction = QAction(
            QCoreApplication.translate("ExtentSelectionPanel", 'Use Canvas Extent'),
            self.btnSelect)
        useLayerExtentAction = QAction(
            QCoreApplication.translate("ExtentSelectionPanel", 'Use Layer Extent…'),
            self.btnSelect)
        selectOnCanvasAction = QAction(
            self.tr('Select Extent on Canvas'), self.btnSelect)

        popupmenu.addAction(useCanvasExtentAction)
        popupmenu.addAction(selectOnCanvasAction)
        popupmenu.addSeparator()
        popupmenu.addAction(useLayerExtentAction)

        selectOnCanvasAction.triggered.connect(self.selectOnCanvas)
        useLayerExtentAction.triggered.connect(self.useLayerExtent)
        useCanvasExtentAction.triggered.connect(self.useCanvasExtent)

        if self.param.flags() & QgsProcessingParameterDefinition.FlagOptional:
            useMincoveringExtentAction = QAction(
                self.tr('Use Min Covering Extent from Input Layers'),
                self.btnSelect)
            useMincoveringExtentAction.triggered.connect(
                self.useMinCoveringExtent)
            popupmenu.addAction(useMincoveringExtentAction)

        popupmenu.exec_(QCursor.pos())

    def useMinCoveringExtent(self):
        self.leText.setText('')

    def useLayerExtent(self):
        dlg = LayerSelectionDialog(self)
        if dlg.exec_():
            layer = dlg.selected_layer()
            self.setValueFromRect(QgsReferencedRectangle(layer.extent(), layer.crs()))

    def useCanvasExtent(self):
        self.setValueFromRect(QgsReferencedRectangle(iface.mapCanvas().extent(),
                                                     iface.mapCanvas().mapSettings().destinationCrs()))

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

        try:
            self.crs = r.crs()
        except:
            self.crs = QgsProject.instance().crs()
        if self.crs.isValid():
            s += ' [' + self.crs.authid() + ']'

        self.leText.setText(s)
        self.tool.reset()
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        self.dialog.showNormal()
        self.dialog.raise_()
        self.dialog.activateWindow()

    def getValue(self):
        if str(self.leText.text()).strip() != '':
            return str(self.leText.text())
        else:
            return None

    def setExtentFromString(self, s):
        self.leText.setText(s)
