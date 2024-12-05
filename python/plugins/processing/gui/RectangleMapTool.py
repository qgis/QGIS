"""
***************************************************************************
    RectangleMapTool.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtGui import QColor
from qgis.core import QgsPointXY, QgsRectangle, QgsWkbTypes
from qgis.gui import QgsMapTool, QgsMapToolEmitPoint, QgsRubberBand


class RectangleMapTool(QgsMapToolEmitPoint):
    rectangleCreated = pyqtSignal()
    deactivated = pyqtSignal()

    def __init__(self, canvas):
        self.canvas = canvas
        QgsMapToolEmitPoint.__init__(self, self.canvas)

        self.rubberBand = QgsRubberBand(
            self.canvas, QgsWkbTypes.GeometryType.PolygonGeometry
        )
        self.rubberBand.setColor(QColor(255, 0, 0, 100))
        self.rubberBand.setWidth(2)

        self.reset()

    def reset(self):
        self.startPoint = self.endPoint = None
        self.isEmittingPoint = False
        self.rubberBand.reset(QgsWkbTypes.GeometryType.PolygonGeometry)

    def canvasPressEvent(self, e):
        self.startPoint = self.toMapCoordinates(e.pos())
        self.endPoint = self.startPoint
        self.isEmittingPoint = True

        self.showRect(self.startPoint, self.endPoint)

    def canvasReleaseEvent(self, e):
        self.isEmittingPoint = False
        if self.rectangle() is not None:
            self.rectangleCreated.emit()

    def canvasMoveEvent(self, e):
        if not self.isEmittingPoint:
            return

        self.endPoint = self.toMapCoordinates(e.pos())
        self.showRect(self.startPoint, self.endPoint)

    def showRect(self, startPoint, endPoint):
        self.rubberBand.reset(QgsWkbTypes.GeometryType.PolygonGeometry)
        if startPoint.x() == endPoint.x() or startPoint.y() == endPoint.y():
            return

        point1 = QgsPointXY(startPoint.x(), startPoint.y())
        point2 = QgsPointXY(startPoint.x(), endPoint.y())
        point3 = QgsPointXY(endPoint.x(), endPoint.y())
        point4 = QgsPointXY(endPoint.x(), startPoint.y())

        self.rubberBand.addPoint(point1, False)
        self.rubberBand.addPoint(point2, False)
        self.rubberBand.addPoint(point3, False)
        # True to update canvas
        self.rubberBand.addPoint(point4, True)
        self.rubberBand.show()

    def rectangle(self):
        if self.startPoint is None or self.endPoint is None:
            return None
        elif (
            self.startPoint.x() == self.endPoint.x()
            or self.startPoint.y() == self.endPoint.y()
        ):
            return None

        return QgsRectangle(self.startPoint, self.endPoint)

    def setRectangle(self, rect):
        if rect == self.rectangle():
            return False

        if rect is None:
            self.reset()
        else:
            self.startPoint = QgsPointXY(rect.xMaximum(), rect.yMaximum())
            self.endPoint = QgsPointXY(rect.xMinimum(), rect.yMinimum())
            self.showRect(self.startPoint, self.endPoint)
        return True

    def deactivate(self):
        QgsMapTool.deactivate(self)
        self.deactivated.emit()
