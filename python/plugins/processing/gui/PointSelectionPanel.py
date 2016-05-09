# -*- coding: utf-8 -*-

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

__author__ = 'Alexander Bruy'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic

from qgis.utils import iface

from processing.gui.PointMapTool import PointMapTool

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class PointSelectionPanel(BASE, WIDGET):

    def __init__(self, dialog, default=None):
        super(PointSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.btnSelect.clicked.connect(self.selectOnCanvas)

        self.dialog = dialog

        canvas = iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()

        self.tool = PointMapTool(canvas)
        self.tool.canvasClicked.connect(self.updatePoint)

        if default:
            tokens = unicode(default).split(',')
            if len(tokens) == 2:
                try:
                    float(tokens[0])
                    float(tokens[1])
                    self.leText.setText(unicode(default))
                except:
                    pass

    def selectOnCanvas(self):
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.tool)
        self.dialog.showMinimized()

    def updatePoint(self, point, button):
        s = '{},{}'.format(point.x(), point.y())

        self.leText.setText(s)
        canvas = iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        self.dialog.showNormal()
        self.dialog.raise_()
        self.dialog.activateWindow()

    def getValue(self):
        if unicode(self.leText.text()).strip() != '':
            return unicode(self.leText.text())
        else:
            return None

    def setPointFromString(self, s):
        self.leText.setText(s)
