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

class ModelerArrowItem(QtGui.QGraphicsPathItem):

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
        


    def contains_point(self, x, y, epsilon):
        p = (x, y)
        min_distance = float(0x7fffffff)
        t = 0.0
        while t < 1.0:
            point = self.path.pointAtPercent(t)
            spline_point = (point.x(), point.y())
            print p, spline_point
            distance = self.distance(p, spline_point)
            if distance < min_distance:
                min_distance = distance
            t += 0.1
        print min_distance, epsilon
        return (min_distance <= epsilon)

    #===========================================================================
    # def boundingRect(self):
    #    return self.path.boundingRect()
    #===========================================================================


    def distance(self, p0, p1):
        a = p1[0] - p0[0]
        b = p1[1] - p0[1]
        return math.sqrt(a * a + b * b)
    
    def startItem(self):
        return self.myStartItem

    def endItem(self):
        return self.myEndItem

    def boundingRect(self):
        #this is a quick fix to avoid arrows not being drawn
        return QtCore.QRectF(0, 0, 4000,4000)

#===============================================================================
#    def shape(self):
#        path = super(ModelerArrowItem, self).shape()
#        path.addPolygon(self.arrowHead)
#        return path
# 
#    def updatePosition(self):
#        line = QtCore.QLineF(self.mapFromItem(self.myStartItem, 0, 0), self.mapFromItem(self.myEndItem, 0, 0))
#        self.setLine(line)
#===============================================================================

    def paint(self, painter, option, widget=None):
        startItem = self.myStartItem
        endItem = self.myEndItem
        myPen = self.pen()
        myPen.setColor(self.myColor)
        painter.setPen(myPen)
        painter.setBrush(self.myColor)

        controlPoints = []
        endPt = self.endItem().getLinkPointForParameter(self.paramIndex)
        startPt = self.startItem().getLinkPointForOutput(self.outputIndex)
        if isinstance(self.startItem().element, GeoAlgorithm):            
            if self.startItem().element.outputs:                
                controlPoints.append(startItem.pos() + startPt) 
                controlPoints.append(startItem.pos() + startPt + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2,0))                
                controlPoints.append(endItem.pos() + endPt - QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2, 0))
                controlPoints.append(endItem.pos() + endPt)                               
                pt = QtCore.QPointF(startItem.pos() + startPt + QtCore.QPointF(-3, -3))
                painter.drawEllipse(pt.x(), pt.y(), 6, 6)
                pt = QtCore.QPointF(endItem.pos() + endPt + QtCore.QPointF(-3, -3))
                painter.drawEllipse(pt.x(), pt.y(), 6, 6)                  
            else: # case where there is a dependency on an algorithm not on an output
                controlPoints.append(startItem.pos() + startPt) 
                controlPoints.append(startItem.pos() + startPt + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2,0))
                controlPoints.append(endItem.pos() + endPt - QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2, 0))
                controlPoints.append(endItem.pos() + endPt)    
        else:            
            controlPoints.append(startItem.pos())
            controlPoints.append(startItem.pos() + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2,0))
            controlPoints.append(endItem.pos() + endPt - QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH /2, 0))
            controlPoints.append(endItem.pos() + endPt)  
            pt = QtCore.QPointF(endItem.pos() + endPt + QtCore.QPointF(-3, -3))
            painter.drawEllipse(pt.x(), pt.y(), 6, 6)                  

        path = QtGui.QPainterPath()
        path.moveTo(controlPoints[0])
        path.cubicTo(*controlPoints[1:])
        painter.strokePath(path, painter.pen())
        self.setPath(path)


