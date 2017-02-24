# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerGraphicItem.py
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
import math

from qgis.PyQt.QtCore import Qt, QPointF, QRectF
from qgis.PyQt.QtGui import QIcon, QFont, QFontMetricsF, QPen, QBrush, QColor, QPolygonF, QPicture, QPainter
from qgis.PyQt.QtWidgets import QGraphicsItem, QMessageBox, QMenu
from qgis.PyQt.QtSvg import QSvgRenderer
from processing.modeler.ModelerAlgorithm import ModelerParameter, Algorithm, ModelerOutput
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerGraphicItem(QGraphicsItem):

    BOX_HEIGHT = 30
    BOX_WIDTH = 200

    def __init__(self, element, model, controls):
        super(ModelerGraphicItem, self).__init__(None)
        self.controls = controls
        self.model = model
        self.element = element
        if isinstance(element, ModelerParameter):
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'input.svg'))
            self.picture = QPicture()
            painter = QPainter(self.picture)
            svg.render(painter)
            self.pixmap = None
            self.text = element.param.description
        elif isinstance(element, ModelerOutput):
            # Output name
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'output.svg'))
            self.picture = QPicture()
            painter = QPainter(self.picture)
            svg.render(painter)
            self.pixmap = None
            self.text = element.description
        else:
            self.text = element.description
            self.pixmap = element.algorithm.getIcon().pixmap(15, 15)
        self.arrows = []
        self.setFlag(QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)
        self.setFlag(QGraphicsItem.ItemSendsGeometryChanges, True)
        self.setZValue(1000)

        if not isinstance(element, ModelerOutput) and controls:
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'edit.svg'))
            picture = QPicture()
            painter = QPainter(picture)
            svg.render(painter)
            pt = QPointF(ModelerGraphicItem.BOX_WIDTH / 2
                         - FlatButtonGraphicItem.WIDTH / 2,
                         ModelerGraphicItem.BOX_HEIGHT / 2
                         - FlatButtonGraphicItem.HEIGHT / 2)
            self.editButton = FlatButtonGraphicItem(picture, pt, self.editElement)
            self.editButton.setParentItem(self)
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'delete.svg'))
            picture = QPicture()
            painter = QPainter(picture)
            svg.render(painter)
            pt = QPointF(ModelerGraphicItem.BOX_WIDTH / 2
                         - FlatButtonGraphicItem.WIDTH / 2,
                         - ModelerGraphicItem.BOX_HEIGHT / 2
                         + FlatButtonGraphicItem.HEIGHT / 2)
            self.deleteButton = FlatButtonGraphicItem(picture, pt,
                                                      self.removeElement)
            self.deleteButton.setParentItem(self)

        if isinstance(element, Algorithm):
            alg = element.algorithm
            if alg.parameters:
                pt = self.getLinkPointForParameter(-1)
                pt = QPointF(0, pt.y())
                if controls:
                    self.inButton = FoldButtonGraphicItem(pt, self.foldInput, self.element.paramsFolded)
                    self.inButton.setParentItem(self)
            if alg.outputs:
                pt = self.getLinkPointForOutput(-1)
                pt = QPointF(0, pt.y())
                if controls:
                    self.outButton = FoldButtonGraphicItem(pt, self.foldOutput, self.element.outputsFolded)
                    self.outButton.setParentItem(self)

    def foldInput(self, folded):
        self.element.paramsFolded = folded
        self.prepareGeometryChange()
        if self.element.algorithm.outputs:
            pt = self.getLinkPointForOutput(-1)
            pt = QPointF(0, pt.y())
            self.outButton.position = pt
        for arrow in self.arrows:
            arrow.updatePath()
        self.update()

    def foldOutput(self, folded):
        self.element.outputsFolded = folded
        self.prepareGeometryChange()
        for arrow in self.arrows:
            arrow.updatePath()
        self.update()

    def addArrow(self, arrow):
        self.arrows.append(arrow)

    def boundingRect(self):
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        unfolded = isinstance(self.element, Algorithm) and not self.element.paramsFolded
        numParams = len(self.element.algorithm.parameters) if unfolded else 0
        unfolded = isinstance(self.element, Algorithm) and not self.element.outputsFolded
        numOutputs = len(self.element.algorithm.outputs) if unfolded else 0

        hUp = fm.height() * 1.2 * (numParams + 2)
        hDown = fm.height() * 1.2 * (numOutputs + 2)
        rect = QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                      -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp,
                      ModelerGraphicItem.BOX_WIDTH + 2,
                      ModelerGraphicItem.BOX_HEIGHT + hDown + hUp)
        return rect

    def mouseDoubleClickEvent(self, event):
        self.editElement()

    def contextMenuEvent(self, event):
        if isinstance(self.element, ModelerOutput):
            return
        popupmenu = QMenu()
        removeAction = popupmenu.addAction('Remove')
        removeAction.triggered.connect(self.removeElement)
        editAction = popupmenu.addAction('Edit')
        editAction.triggered.connect(self.editElement)
        if isinstance(self.element, Algorithm):
            if not self.element.active:
                removeAction = popupmenu.addAction('Activate')
                removeAction.triggered.connect(self.activateAlgorithm)
            else:
                deactivateAction = popupmenu.addAction('Deactivate')
                deactivateAction.triggered.connect(self.deactivateAlgorithm)
        popupmenu.exec_(event.screenPos())

    def deactivateAlgorithm(self):
        self.model.deactivateAlgorithm(self.element.name)
        self.model.updateModelerView()

    def activateAlgorithm(self):
        if self.model.activateAlgorithm(self.element.name):
            self.model.updateModelerView()
        else:
            QMessageBox.warning(None, 'Could not activate Algorithm',
                                'The selected algorithm depends on other currently non-active algorithms.\n'
                                'Activate them them before trying to activate it.')

    def editElement(self):
        if isinstance(self.element, ModelerParameter):
            dlg = ModelerParameterDefinitionDialog(self.model,
                                                   param=self.element.param)
            dlg.exec_()
            if dlg.param is not None:
                self.model.updateParameter(dlg.param)
                self.element.param = dlg.param
                self.text = dlg.param.description
                self.update()
        elif isinstance(self.element, Algorithm):
            dlg = self.element.algorithm.getCustomModelerParametersDialog(self.model, self.element.name)
            if not dlg:
                dlg = ModelerParametersDialog(self.element.algorithm, self.model, self.element.name)
            dlg.exec_()
            if dlg.alg is not None:
                dlg.alg.name = self.element.name
                self.model.updateAlgorithm(dlg.alg)
                self.model.updateModelerView()

    def removeElement(self):
        if isinstance(self.element, ModelerParameter):
            if not self.model.removeParameter(self.element.param.name):
                QMessageBox.warning(None, 'Could not remove element',
                                    'Other elements depend on the selected one.\n'
                                    'Remove them before trying to remove it.')
            else:
                self.model.updateModelerView()
        elif isinstance(self.element, Algorithm):
            if not self.model.removeAlgorithm(self.element.name):
                QMessageBox.warning(None, 'Could not remove element',
                                    'Other elements depend on the selected one.\n'
                                    'Remove them before trying to remove it.')
            else:
                self.model.updateModelerView()

    def getAdjustedText(self, text):
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        w = fm.width(text)
        if w < self.BOX_WIDTH - 25 - FlatButtonGraphicItem.WIDTH:
            return text

        text = text[0:-3] + '...'
        w = fm.width(text)
        while w > self.BOX_WIDTH - 25 - FlatButtonGraphicItem.WIDTH:
            text = text[0:-4] + '...'
            w = fm.width(text)
        return text

    def paint(self, painter, option, widget=None):
        rect = QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2.0,
                      -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2.0,
                      ModelerGraphicItem.BOX_WIDTH + 2,
                      ModelerGraphicItem.BOX_HEIGHT + 2)

        if isinstance(self.element, ModelerParameter):
            color = QColor(238, 242, 131)
            stroke = QColor(234, 226, 118)
            selected = QColor(116, 113, 68)
        elif isinstance(self.element, Algorithm):
            color = QColor(255, 255, 255)
            stroke = Qt.gray
            selected = QColor(50, 50, 50)
        else:
            color = QColor(172, 196, 114)
            stroke = QColor(90, 140, 90)
            selected = QColor(42, 65, 42)
        if self.isSelected():
            stroke = selected
            color = color.darker(110)
        painter.setPen(QPen(stroke, 0)) # 0 width "cosmetic" pen
        painter.setBrush(QBrush(color, Qt.SolidPattern))
        painter.drawRect(rect)
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        painter.setFont(font)
        painter.setPen(QPen(Qt.black))
        text = self.getAdjustedText(self.text)
        if isinstance(self.element, Algorithm) and not self.element.active:
            painter.setPen(QPen(Qt.gray))
            text = text + "\n(deactivated)"
        fm = QFontMetricsF(font)
        text = self.getAdjustedText(self.text)
        h = fm.ascent()
        pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, ModelerGraphicItem.BOX_HEIGHT / 2.0 - h + 1)
        painter.drawText(pt, text)
        painter.setPen(QPen(Qt.black))
        if isinstance(self.element, Algorithm):
            h = -(fm.height() * 1.2)
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'In')
            i = 1
            if not self.element.paramsFolded:
                for param in self.element.algorithm.parameters:
                    if not param.hidden:
                        text = self.getAdjustedText(param.description)
                        h = -(fm.height() * 1.2) * (i + 1)
                        h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
                        pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2
                                     + 33, h)
                        painter.drawText(pt, text)
                        i += 1
            h = fm.height() * 1.1
            h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
            pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'Out')
            if not self.element.outputsFolded:
                for i, out in enumerate(self.element.algorithm.outputs):
                    text = self.getAdjustedText(out.description)
                    h = fm.height() * 1.2 * (i + 2)
                    h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
                    pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2
                                 + 33, h)
                    painter.drawText(pt, text)
        if self.pixmap:
            painter.drawPixmap(-(ModelerGraphicItem.BOX_WIDTH / 2.0) + 3, -8,
                               self.pixmap)
        elif self.picture:
            painter.drawPicture(-(ModelerGraphicItem.BOX_WIDTH / 2.0) + 3, -8,
                                self.picture)

    def getLinkPointForParameter(self, paramIndex):
        offsetX = 25
        if isinstance(self.element, Algorithm) and self.element.paramsFolded:
            paramIndex = -1
            offsetX = 17
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        if isinstance(self.element, Algorithm):
            h = -(fm.height() * 1.2) * (paramIndex + 2) - fm.height() / 2.0 + 8
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0
        else:
            h = 0
        return QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + offsetX, h)

    def getLinkPointForOutput(self, outputIndex):
        if isinstance(self.element, Algorithm) and self.element.algorithm.outputs:
            outputIndex = (outputIndex if not self.element.outputsFolded else -1)
            text = self.getAdjustedText(self.element.algorithm.outputs[outputIndex].description)
            font = QFont('Verdana', 8)
            font.setPixelSize(12)
            fm = QFontMetricsF(font)
            w = fm.width(text)
            h = fm.height() * 1.2 * (outputIndex + 1) + fm.height() / 2.0
            y = h + ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            x = (-ModelerGraphicItem.BOX_WIDTH / 2 + 33 + w
                 + 5 if not self.element.outputsFolded else 10)
            return QPointF(x, y)
        else:
            return QPointF(0, 0)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemPositionHasChanged:
            for arrow in self.arrows:
                arrow.updatePath()
            self.element.pos = self.pos()

        return value

    def polygon(self):
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        hUp = fm.height() * 1.2 * (len(self.element.parameters) + 2)
        hDown = fm.height() * 1.2 * (len(self.element.outputs) + 2)
        pol = QPolygonF([
            QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                    -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp),
            QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                    (ModelerGraphicItem.BOX_HEIGHT + 2) / 2 + hDown),
            QPointF((ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                    (ModelerGraphicItem.BOX_HEIGHT + 2) / 2 + hDown),
            QPointF((ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                    -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp),
            QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                    -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp)
        ])
        return pol


class FlatButtonGraphicItem(QGraphicsItem):

    WIDTH = 16
    HEIGHT = 16

    def __init__(self, picture, position, action):
        super(FlatButtonGraphicItem, self).__init__(None)
        self.setAcceptHoverEvents(True)
        self.setFlag(QGraphicsItem.ItemIsMovable, False)
        self.picture = picture
        self.position = position
        self.isIn = False
        self.action = action

    def mousePressEvent(self, event):
        self.action()

    def paint(self, painter, option, widget=None):
        pt = QPointF(-math.floor(self.WIDTH / 2), -math.floor(self.HEIGHT / 2)) + self.position
        rect = QRectF(pt.x(), pt.y(), self.WIDTH, self.HEIGHT)
        if self.isIn:
            painter.setPen(QPen(Qt.transparent, 1))
            painter.setBrush(QBrush(QColor(55, 55, 55, 33),
                                    Qt.SolidPattern))
        else:
            painter.setPen(QPen(Qt.transparent, 1))
            painter.setBrush(QBrush(Qt.transparent,
                                    Qt.SolidPattern))
        painter.drawRect(rect)
        painter.drawPicture(pt.x(), pt.y(), self.picture)

    def boundingRect(self):
        rect = QRectF(self.position.x() - math.floor(self.WIDTH / 2),
                      self.position.y() - math.floor(self.HEIGHT / 2),
                      self.WIDTH,
                      self.HEIGHT)
        return rect

    def hoverEnterEvent(self, event):
        self.isIn = True
        self.update()

    def hoverLeaveEvent(self, event):
        self.isIn = False
        self.update()


class FoldButtonGraphicItem(FlatButtonGraphicItem):

    WIDTH = 11
    HEIGHT = 11

    def __init__(self, position, action, folded):
        plus = QPicture()
        minus = QPicture()

        svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'plus.svg'))
        painter = QPainter(plus)
        svg.render(painter)
        svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'minus.svg'))
        painter = QPainter(minus)
        svg.render(painter)

        self.pictures = {True: plus,
                         False: minus}

        self.folded = folded
        picture = self.pictures[self.folded]
        super(FoldButtonGraphicItem, self).__init__(picture, position, action)

    def mousePressEvent(self, event):
        self.folded = not self.folded
        self.picture = self.pictures[self.folded]
        self.action(self.folded)
