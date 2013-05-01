from PyQt4 import QtCore, QtGui
import math
from sextante.modeler.ModelerGraphicItem import ModelerGraphicItem
from sextante.core.GeoAlgorithm import GeoAlgorithm

#portions of this code have been taken and adapted from PyQt examples, released under the following license terms

#############################################################################
##
## Copyright (C) 2010 Riverbank Computing Limited.
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
## All rights reserved.
##
## This file is part of the examples of PyQt.
##
## $QT_BEGIN_LICENSE:BSD$
## You may use this file under the terms of the BSD license as follows:
##
## "Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
##   * Redistributions of source code must retain the above copyright
##     notice, this list of conditions and the following disclaimer.
##   * Redistributions in binary form must reproduce the above copyright
##     notice, this list of conditions and the following disclaimer in
##     the documentation and/or other materials provided with the
##     distribution.
##   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
##     the names of its contributors may be used to endorse or promote
##     products derived from this software without specific prior written
##     permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
## LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
## $QT_END_LICENSE$
##
#############################################################################

class ModelerArrowItem(QtGui.QGraphicsLineItem):

    def __init__(self, startItem, outputIndex, endItem, paramIndex ,parent=None, scene=None):
        super(ModelerArrowItem, self).__init__(parent, scene)
        self.arrowHead = QtGui.QPolygonF()
        self.paramIndex = paramIndex
        self.outputIndex = outputIndex
        self.myStartItem = startItem
        self.myEndItem = endItem
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, False)
        self.myColor = QtCore.Qt.gray
        self.setPen(QtGui.QPen(self.myColor, 1, QtCore.Qt.SolidLine,
                QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin))
        self.setZValue(0)


    def startItem(self):
        return self.myStartItem

    def endItem(self):
        return self.myEndItem

    def boundingRect(self):
        #this is a quick fix to avoid arrows not being drawn
        return QtCore.QRectF(0, 0, 4000,4000)

    def shape(self):
        path = super(ModelerArrowItem, self).shape()
        path.addPolygon(self.arrowHead)
        return path

    def updatePosition(self):
        line = QtCore.QLineF(self.mapFromItem(self.myStartItem, 0, 0), self.mapFromItem(self.myEndItem, 0, 0))
        self.setLine(line)

    def paint(self, painter, option, widget=None):
        myStartItem = self.myStartItem
        myEndItem = self.myEndItem
        myPen = self.pen()
        myPen.setColor(self.myColor)
        arrowSize = 6.0
        painter.setPen(myPen)
        painter.setBrush(self.myColor)

        if isinstance(self.startItem().element, GeoAlgorithm):
            if self.startItem().element.outputs:
                endPt = self.endItem().getLinkPointForParameter(self.paramIndex)
                startPt = self.startItem().getLinkPointForOutput(self.outputIndex)
                arrowLine = QtCore.QLineF(myEndItem.pos() + endPt - QtCore.QPointF(endPt.x() + ModelerGraphicItem.BOX_WIDTH /2, 0), myEndItem.pos() + endPt)
                painter.drawLine(arrowLine)
                tailLine = QtCore.QLineF(myStartItem.pos() + startPt + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2 - startPt.x(),0), myStartItem.pos() + startPt)
                painter.drawLine(tailLine)
                pt = QtCore.QPointF(myStartItem.pos() + startPt + QtCore.QPointF(- 2, -2))
                rect = QtCore.QRectF(pt.x(), pt.y(), 4, 4)
                painter.fillRect(rect, QtCore.Qt.gray)
                line = QtCore.QLineF(myStartItem.pos() + startPt + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2 - startPt.x(),0),
                                 myEndItem.pos() + endPt - QtCore.QPointF(endPt.x() + ModelerGraphicItem.BOX_WIDTH /2, 0))
            else: # case where there is a dependency on an algorithm not on an output
                endPolygon = myEndItem.polygon()
                p1 = endPolygon.first() + myEndItem.pos()
                line = QtCore.QLineF(myStartItem.pos(), myEndItem.pos())
                intersectPoint = QtCore.QPointF()
                for i in endPolygon:
                    p2 = i + myEndItem.pos()
                    polyLine = QtCore.QLineF(p1, p2)
                    intersectType = polyLine.intersect(line, intersectPoint)
                    if intersectType == QtCore.QLineF.BoundedIntersection:
                        break
                    p1 = p2

                self.setLine(QtCore.QLineF(intersectPoint, myStartItem.pos()))
                line = self.line()
                if line.length() == 0: #division by zero might occur if arrow has no length
                    return
                angle = math.acos(line.dx() / line.length())
                if line.dy() >= 0:
                    angle = (math.pi * 2.0) - angle

                arrowP1 = line.p1() + QtCore.QPointF(math.sin(angle + math.pi / 3.0) * arrowSize,
                                                math.cos(angle + math.pi / 3) * arrowSize)
                arrowP2 = line.p1() + QtCore.QPointF(math.sin(angle + math.pi - math.pi / 3.0) * arrowSize,
                                                math.cos(angle + math.pi - math.pi / 3.0) * arrowSize)

                self.arrowHead.clear()
                for point in [line.p1(), arrowP1, arrowP2]:
                    self.arrowHead.append(point)

                painter.drawLine(line)
                painter.drawPolygon(self.arrowHead)
                return;
        else:
            endPt = self.endItem().getLinkPointForParameter(self.paramIndex)
            arrowLine = QtCore.QLineF(myEndItem.pos() + endPt - QtCore.QPointF(endPt.x() + ModelerGraphicItem.BOX_WIDTH /2, 0), myEndItem.pos() + endPt)
            painter.drawLine(arrowLine)
            line = QtCore.QLineF(myStartItem.pos(),
                             myEndItem.pos() + endPt - QtCore.QPointF(endPt.x() + ModelerGraphicItem.BOX_WIDTH /2, 0))

        self.setLine(line);

        if line.length() == 0: #division by zero might occur if arrow has no length
            return

        angle = math.acos(line.dx() / line.length())
        if line.dy() >= 0:
            angle = (math.pi * 2.0) - angle

        arrowP1 = arrowLine.p2() + QtCore.QPointF(-math.cos(math.pi / 9.0) * arrowSize,
                                        math.sin(math.pi / 9.0) * arrowSize)
        arrowP2 = arrowLine.p2() + QtCore.QPointF(-math.cos(math.pi / 9.0) * arrowSize,
                                        -math.sin(math.pi / 9.0) * arrowSize)

        self.arrowHead.clear()
        for point in [arrowLine.p2(), arrowP1, arrowP2]:
            self.arrowHead.append(point)

        painter.drawLine(line)
        painter.drawPolygon(self.arrowHead)

