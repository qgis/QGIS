"""
***************************************************************************
    PointMapTool.py
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

from qgis.PyQt.QtCore import Qt, pyqtSignal

from qgis.gui import QgsMapToolEmitPoint


class PointMapTool(QgsMapToolEmitPoint):
    complete = pyqtSignal()

    def __init__(self, canvas):
        QgsMapToolEmitPoint.__init__(self, canvas)

        self.canvas = canvas
        self.cursor = Qt.CursorShape.CrossCursor

    def activate(self):
        self.canvas.setCursor(self.cursor)

    def canvasReleaseEvent(self, event):
        self.complete.emit()
