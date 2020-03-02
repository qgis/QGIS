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

from qgis.gui import (
    QgsModelGraphicsScene,
    QgsModelComponentGraphicItem
)
from qgis.PyQt.QtCore import Qt, QPointF
from qgis.PyQt.QtWidgets import QApplication, QGraphicsPathItem, QGraphicsItem
from qgis.PyQt.QtGui import QPen, QPainterPath, QPolygonF, QPainter, QPalette


class ModelerArrowItem(QGraphicsPathItem):

    def __init__(self, startItem, start_edge, startIndex, endItem, end_edge, endIndex,
                 parent=None):
        super(ModelerArrowItem, self).__init__(parent)
        self.arrowHead = QPolygonF()
        self.endIndex = endIndex
        self.startIndex = startIndex
        self.start_edge = start_edge
        self.startItem = startItem
        self.endItem = endItem
        self.end_edge = end_edge
        self.endPoints = []
        self.setFlag(QGraphicsItem.ItemIsSelectable, False)
        self.myColor = QApplication.palette().color(QPalette.WindowText)
        self.myColor.setAlpha(150)
        self.setPen(QPen(self.myColor, 4, Qt.SolidLine,
                         Qt.RoundCap, Qt.RoundJoin))
        self.setZValue(QgsModelGraphicsScene.ArrowLink)

    def setPenStyle(self, style):
        pen = self.pen()
        pen.setStyle(style)
        self.setPen(pen)
        self.update()

    def updatePath(self):
        self.endPoints = []
        controlPoints = []

        # is there a fixed start or end point?
        startPt = None
        if self.start_edge is not None and self.startIndex is not None:
            startPt = self.startItem.linkPoint(self.start_edge, self.startIndex)
        endPt = None
        if self.end_edge is not None and self.endIndex is not None:
            endPt = self.endItem.linkPoint(self.end_edge, self.endIndex)

        if startPt is None:
            # find closest edge
            if endPt is None:
                pt, edge = self.startItem.calculateAutomaticLinkPoint(self.endItem)
            else:
                pt, edge = self.startItem.calculateAutomaticLinkPoint(endPt + self.endItem.pos())
            controlPoints.append(pt)
            self.endPoints.append(pt)
            if edge == Qt.LeftEdge:
                controlPoints.append(pt - QPointF(50, 0))
            elif edge == Qt.RightEdge:
                controlPoints.append(pt + QPointF(50, 0))
            elif edge == Qt.BottomEdge:
                controlPoints.append(pt + QPointF(0, 30))
            else:
                controlPoints.append(pt + QPointF(0, -30))
        else:
            self.endPoints.append(self.startItem.pos() + startPt)
            controlPoints.append(self.startItem.pos() + startPt)
            controlPoints.append(self.startItem.pos() + startPt +
                                 QPointF(self.startItem.component().size().width() / 3, 0))

        if endPt is None:
            # find closest edge
            if startPt is None:
                pt, edge = self.endItem.calculateAutomaticLinkPoint(self.startItem)
            else:
                pt, edge = self.endItem.calculateAutomaticLinkPoint(startPt + self.startItem.pos())
            if edge == Qt.LeftEdge:
                controlPoints.append(pt - QPointF(50, 0))
            elif edge == Qt.RightEdge:
                controlPoints.append(pt + QPointF(50, 0))
            elif edge == Qt.BottomEdge:
                controlPoints.append(pt + QPointF(0, 30))
            else:
                controlPoints.append(pt + QPointF(0, -30))
            controlPoints.append(pt)
            self.endPoints.append(pt)
        else:
            self.endPoints.append(self.endItem.pos() + endPt)
            controlPoints.append(self.endItem.pos() + endPt -
                                 QPointF(self.endItem.component().size().width() / 3, 0))
            controlPoints.append(self.endItem.pos() + endPt)

        path = QPainterPath()
        path.moveTo(controlPoints[0])
        path.cubicTo(*controlPoints[1:])
        self.setPath(path)

    def paint(self, painter, option, widget=None):
        color = self.myColor

        if self.startItem.state() == QgsModelComponentGraphicItem.Selected or self.endItem.state() == QgsModelComponentGraphicItem.Selected:
            color.setAlpha(220)
        elif self.startItem.state() == QgsModelComponentGraphicItem.Hover or self.endItem.state() == QgsModelComponentGraphicItem.Hover:
            color.setAlpha(150)
        else:
            color.setAlpha(80)

        myPen = self.pen()
        myPen.setColor(color)
        myPen.setWidth(1)
        painter.setPen(myPen)
        painter.setBrush(color)
        painter.setRenderHint(QPainter.Antialiasing)

        for point in self.endPoints:
            painter.drawEllipse(point, 3.0, 3.0)

        painter.setBrush(Qt.NoBrush)
        painter.drawPath(self.path())
