# -*- coding: utf-8 -*-

"""
***************************************************************************
    Portions of this code have been taken and adapted from PyQt
    examples, released under the following license terms

#############################################################################
#
# Copyright (C) 2010 Riverbank Computing Limited.
# Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
#
# This file is part of the examples of PyQt.
#
# $QT_BEGIN_LICENSE:BSD$
# You may use this file under the terms of the BSD license as follows:
#
# "Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
#     the names of its contributors may be used to endorse or promote
#     products derived from this software without specific prior written
#     permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
# $QT_END_LICENSE$
#
***************************************************************************
"""

from qgis.core import (QgsProcessingModelAlgorithm,
                       QgsProcessingModelChildAlgorithm,
                       QgsProcessingModelParameter)
from qgis.PyQt.QtCore import Qt, QPointF
from qgis.PyQt.QtWidgets import QApplication, QGraphicsPathItem, QGraphicsItem
from qgis.PyQt.QtGui import QPen, QPainterPath, QPolygonF, QPainter, QPalette
from processing.modeler.ModelerGraphicItem import ModelerGraphicItem


class ModelerArrowItem(QGraphicsPathItem):

    def __init__(self, startItem, startIndex, endItem, endIndex,
                 parent=None, scene=None):
        super(ModelerArrowItem, self).__init__(parent)
        self.arrowHead = QPolygonF()
        self.endIndex = endIndex
        self.startIndex = startIndex
        self.startItem = startItem
        self.endItem = endItem
        self.endPoints = []
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.myColor = QApplication.palette().color(QPalette.WindowText)
        self.myColor.setAlpha(150)
        self.setPen(QPen(self.myColor, 1, Qt.SolidLine,
                         Qt.RoundCap, Qt.RoundJoin))
        self.setZValue(0)

    def setPenStyle(self, style):
        pen = self.pen()
        pen.setStyle(style)
        self.setPen(pen)
        self.update()

    def updatePath(self):
        self.endPoints = []
        controlPoints = []
        endPt = self.endItem.getLinkPointForParameter(self.endIndex)
        if isinstance(self.startItem.element, QgsProcessingModelParameter):
            startPt = self.startItem.getLinkPointForParameter(self.startIndex)
        else:
            startPt = self.startItem.getLinkPointForOutput(self.startIndex)
        if isinstance(self.endItem.element, QgsProcessingModelParameter):
            endPt = self.endItem.getLinkPointForParameter(self.startIndex)

        if isinstance(self.startItem.element, QgsProcessingModelChildAlgorithm):
            if self.startIndex != -1:
                controlPoints.append(self.startItem.pos() + startPt)
                controlPoints.append(self.startItem.pos() + startPt +
                                     QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
                controlPoints.append(self.endItem.pos() + endPt -
                                     QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
                controlPoints.append(self.endItem.pos() + endPt)
                pt = QPointF(self.startItem.pos() + startPt + QPointF(-3, -3))
                self.endPoints.append(pt)
                pt = QPointF(self.endItem.pos() + endPt + QPointF(-3, -3))
                self.endPoints.append(pt)
            else:
                # Case where there is a dependency on an algorithm not
                # on an output
                controlPoints.append(self.startItem.pos() + startPt)
                controlPoints.append(self.startItem.pos() + startPt +
                                     QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
                controlPoints.append(self.endItem.pos() + endPt -
                                     QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
                controlPoints.append(self.endItem.pos() + endPt)
        else:
            controlPoints.append(self.startItem.pos())
            controlPoints.append(self.startItem.pos() +
                                 QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
            controlPoints.append(self.endItem.pos() + endPt -
                                 QPointF(ModelerGraphicItem.BOX_WIDTH / 3, 0))
            controlPoints.append(self.endItem.pos() + endPt)
            pt = QPointF(self.endItem.pos() + endPt + QPointF(-3, -3))
            self.endPoints.append(pt)
        path = QPainterPath()
        path.moveTo(controlPoints[0])
        path.cubicTo(*controlPoints[1:])
        self.setPath(path)

    def paint(self, painter, option, widget=None):
        color = self.myColor

        if self.startItem.isSelected() or self.endItem.isSelected():
            color.setAlpha(220)
        elif self.startItem.hover_over_item or self.endItem.hover_over_item:
            color.setAlpha(150)
        else:
            color.setAlpha(80)

        myPen = self.pen()
        myPen.setColor(color)
        painter.setPen(myPen)
        painter.setBrush(color)
        painter.setRenderHint(QPainter.Antialiasing)

        for point in self.endPoints:
            painter.drawEllipse(point.x(), point.y(), 6, 6)

        painter.setBrush(Qt.NoBrush)
        painter.drawPath(self.path())
