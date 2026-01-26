"""
***************************************************************************
    PointSelectionPanel.py
    ---------------------
    Date                 : February 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "February 2016"
__copyright__ = "(C) 2016, Alexander Bruy"

import os
import warnings

from qgis.core import QgsProject, QgsReferencedPointXY, QgsPointXY
from qgis.PyQt import uic

from qgis.utils import iface

from processing.gui.PointMapTool import PointMapTool

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "widgetBaseSelector.ui")
    )


class PointSelectionPanel(BASE, WIDGET):

    def __init__(self, dialog, default=None):
        super().__init__(None)
        self.setupUi(self)

        self.btnSelect.clicked.connect(self.selectOnCanvas)

        self.dialog = dialog
        self.crs = QgsProject.instance().crs()

        if iface is not None:
            canvas = iface.mapCanvas()
            self.prevMapTool = canvas.mapTool()

            self.tool = PointMapTool(canvas)
            self.tool.canvasClicked.connect(self.updatePoint)
            self.tool.complete.connect(self.pointPicked)
        else:
            self.prevMapTool = None
            self.tool = None

        if default:
            tokens = str(default).split(",")
            if len(tokens) == 2:
                try:
                    float(tokens[0])
                    float(tokens[1])
                    self.leText.setText(str(default))
                except:
                    pass

    def selectOnCanvas(self):
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.tool)
        self.dialog.showMinimized()

    def updatePoint(self, point, button):
        s = f"{point.x()},{point.y()}"
        self.crs = QgsProject.instance().crs()
        if self.crs.isValid():
            s += " [" + self.crs.authid() + "]"
        self.leText.setText(s)

    def pointPicked(self):
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        self.dialog.showNormal()
        self.dialog.raise_()
        self.dialog.activateWindow()

    def getValue(self):
        if str(self.leText.text()).strip() != "":
            return str(self.leText.text())
        else:
            return None

    def setPointFromString(self, s):
        self.leText.setText(s)
