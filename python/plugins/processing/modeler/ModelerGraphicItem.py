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
from qgis.PyQt.QtGui import QFont, QFontMetricsF, QPen, QBrush, QColor, QPolygonF, QPicture, QPainter
from qgis.PyQt.QtWidgets import QGraphicsItem, QMessageBox, QMenu
from qgis.PyQt.QtSvg import QSvgRenderer
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingModelParameter,
                       QgsProcessingModelOutput,
                       QgsProcessingModelChildAlgorithm,
                       QgsProcessingModelAlgorithm)
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerGraphicItem(QGraphicsItem):

    BOX_HEIGHT = 30
    BOX_WIDTH = 200

    def __init__(self, element, model, controls, scene=None):
        super(ModelerGraphicItem, self).__init__(None)
        self.controls = controls
        self.model = model
        self.scene = scene
        self.element = element
        if isinstance(element, QgsProcessingModelParameter):
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'input.svg'))
            self.picture = QPicture()
            painter = QPainter(self.picture)
            svg.render(painter)
            self.pixmap = None
            self.text = self.model.parameterDefinition(element.parameterName()).description()
        elif isinstance(element, QgsProcessingModelOutput):
            # Output name
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'output.svg'))
            self.picture = QPicture()
            painter = QPainter(self.picture)
            svg.render(painter)
            self.pixmap = None
            self.text = element.name()
        else:
            self.text = element.description()
            self.pixmap = element.algorithm().icon().pixmap(15, 15)
        self.arrows = []
        self.setFlag(QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.ItemIsSelectable, True)
        self.setFlag(QGraphicsItem.ItemSendsGeometryChanges, True)
        self.setZValue(1000)

        if not isinstance(element, QgsProcessingModelOutput) and controls:
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'edit.svg'))
            picture = QPicture()
            painter = QPainter(picture)
            svg.render(painter)
            pt = QPointF(ModelerGraphicItem.BOX_WIDTH / 2 -
                         FlatButtonGraphicItem.WIDTH / 2,
                         ModelerGraphicItem.BOX_HEIGHT / 2 -
                         FlatButtonGraphicItem.HEIGHT / 2)
            self.editButton = FlatButtonGraphicItem(picture, pt, self.editElement)
            self.editButton.setParentItem(self)
            svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'delete.svg'))
            picture = QPicture()
            painter = QPainter(picture)
            svg.render(painter)
            pt = QPointF(ModelerGraphicItem.BOX_WIDTH / 2 -
                         FlatButtonGraphicItem.WIDTH / 2,
                         FlatButtonGraphicItem.HEIGHT / 2 -
                         ModelerGraphicItem.BOX_HEIGHT / 2)
            self.deleteButton = FlatButtonGraphicItem(picture, pt,
                                                      self.removeElement)
            self.deleteButton.setParentItem(self)

        if isinstance(element, QgsProcessingModelChildAlgorithm):
            alg = element.algorithm()
            if [a for a in alg.parameterDefinitions() if not a.isDestination()]:
                pt = self.getLinkPointForParameter(-1)
                pt = QPointF(0, pt.y())
                if controls:
                    self.inButton = FoldButtonGraphicItem(pt, self.foldInput, self.element.parametersCollapsed())
                    self.inButton.setParentItem(self)
            if alg.outputDefinitions():
                pt = self.getLinkPointForOutput(-1)
                pt = QPointF(0, pt.y())
                if controls:
                    self.outButton = FoldButtonGraphicItem(pt, self.foldOutput, self.element.outputsCollapsed())
                    self.outButton.setParentItem(self)

    def foldInput(self, folded):
        self.element.setParametersCollapsed(folded)
        #also need to update the model's stored component
        self.model.childAlgorithm(self.element.childId()).setParametersCollapsed(folded)
        self.prepareGeometryChange()
        if self.element.algorithm().outputDefinitions():
            pt = self.getLinkPointForOutput(-1)
            pt = QPointF(0, pt.y())
            self.outButton.position = pt
        for arrow in self.arrows:
            arrow.updatePath()
        self.update()

    def foldOutput(self, folded):
        self.element.setOutputsCollapsed(folded)
        # also need to update the model's stored component
        self.model.childAlgorithm(self.element.childId()).setOutputsCollapsed(folded)
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
        unfolded = isinstance(self.element, QgsProcessingModelChildAlgorithm) and not self.element.parametersCollapsed()
        numParams = len([a for a in self.element.algorithm().parameterDefinitions() if not a.isDestination()]) if unfolded else 0
        unfolded = isinstance(self.element, QgsProcessingModelChildAlgorithm) and not self.element.outputsCollapsed()
        numOutputs = len(self.element.algorithm().outputDefinitions()) if unfolded else 0

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
        if isinstance(self.element, QgsProcessingModelOutput):
            return
        popupmenu = QMenu()
        removeAction = popupmenu.addAction('Remove')
        removeAction.triggered.connect(self.removeElement)
        editAction = popupmenu.addAction('Edit')
        editAction.triggered.connect(self.editElement)
        if isinstance(self.element, QgsProcessingModelChildAlgorithm):
            if not self.element.isActive():
                removeAction = popupmenu.addAction('Activate')
                removeAction.triggered.connect(self.activateAlgorithm)
            else:
                deactivateAction = popupmenu.addAction('Deactivate')
                deactivateAction.triggered.connect(self.deactivateAlgorithm)
        popupmenu.exec_(event.screenPos())

    def deactivateAlgorithm(self):
        self.model.deactivateChildAlgorithm(self.element.childId())
        self.scene.dialog.repaintModel()

    def activateAlgorithm(self):
        if self.model.activateChildAlgorithm(self.element.childId()):
            self.scene.dialog.repaintModel()
        else:
            QMessageBox.warning(None, 'Could not activate Algorithm',
                                'The selected algorithm depends on other currently non-active algorithms.\n'
                                'Activate them them before trying to activate it.')

    def editElement(self):
        if isinstance(self.element, QgsProcessingModelParameter):
            dlg = ModelerParameterDefinitionDialog(self.model,
                                                   param=self.model.parameterDefinition(self.element.parameterName()))
            if dlg.exec_() and dlg.param is not None:
                self.model.removeModelParameter(self.element.parameterName())
                self.element.setParameterName(dlg.param.name())
                self.element.setDescription(dlg.param.name())
                self.model.addModelParameter(dlg.param, self.element)
                self.text = dlg.param.description()
                self.scene.dialog.repaintModel()
        elif isinstance(self.element, QgsProcessingModelChildAlgorithm):
            dlg = None
            try:
                dlg = self.element.algorithm().getCustomModelerParametersDialog(self.model, self.element.childId())
            except:
                pass
            if not dlg:
                dlg = ModelerParametersDialog(self.element.algorithm(), self.model, self.element.childId())
            if dlg.exec_():
                alg = dlg.createAlgorithm()
                alg.setChildId(self.element.childId())
                self.updateAlgorithm(alg)
                self.scene.dialog.repaintModel()

    def updateAlgorithm(self, alg):
        existing_child = self.model.childAlgorithm(alg.childId())
        alg.setPosition(existing_child.position())
        alg.setParametersCollapsed(existing_child.parametersCollapsed())
        alg.setOutputsCollapsed(existing_child.outputsCollapsed())
        for i, out in enumerate(alg.modelOutputs().keys()):
            alg.modelOutput(out).setPosition(alg.modelOutput(out).position() or
                                             alg.position() + QPointF(
                ModelerGraphicItem.BOX_WIDTH,
                (i + 1.5) * ModelerGraphicItem.BOX_HEIGHT))
        self.model.setChildAlgorithm(alg)

    def removeElement(self):
        if isinstance(self.element, QgsProcessingModelParameter):
            if self.model.childAlgorithmsDependOnParameter(self.element.parameterName()):
                QMessageBox.warning(None, 'Could not remove input',
                                    'Algorithms depend on the selected input.\n'
                                    'Remove them before trying to remove it.')
            elif self.model.otherParametersDependOnParameter(self.element.parameterName()):
                QMessageBox.warning(None, 'Could not remove input',
                                    'Other inputs depend on the selected input.\n'
                                    'Remove them before trying to remove it.')
            else:
                self.model.removeModelParameter(self.element.parameterName())
                self.scene.dialog.haschanged = True
                self.scene.dialog.repaintModel()
        elif isinstance(self.element, QgsProcessingModelChildAlgorithm):
            if not self.model.removeChildAlgorithm(self.element.childId()):
                QMessageBox.warning(None, 'Could not remove element',
                                    'Other elements depend on the selected one.\n'
                                    'Remove them before trying to remove it.')
            else:
                self.scene.dialog.haschanged = True
                self.scene.dialog.repaintModel()

    def getAdjustedText(self, text):
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        w = fm.width(text)
        if w < self.BOX_WIDTH - 25 - FlatButtonGraphicItem.WIDTH:
            return text

        text = text[0:-3] + '…'
        w = fm.width(text)
        while w > self.BOX_WIDTH - 25 - FlatButtonGraphicItem.WIDTH:
            text = text[0:-4] + '…'
            w = fm.width(text)
        return text

    def paint(self, painter, option, widget=None):
        rect = QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2.0,
                      -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2.0,
                      ModelerGraphicItem.BOX_WIDTH + 2,
                      ModelerGraphicItem.BOX_HEIGHT + 2)

        if isinstance(self.element, QgsProcessingModelParameter):
            color = QColor(238, 242, 131)
            stroke = QColor(234, 226, 118)
            selected = QColor(116, 113, 68)
        elif isinstance(self.element, QgsProcessingModelChildAlgorithm):
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
        painter.setPen(QPen(stroke, 0))  # 0 width "cosmetic" pen
        painter.setBrush(QBrush(color, Qt.SolidPattern))
        painter.drawRect(rect)
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        painter.setFont(font)
        painter.setPen(QPen(Qt.black))
        text = self.getAdjustedText(self.text)
        if isinstance(self.element, QgsProcessingModelChildAlgorithm) and not self.element.isActive():
            painter.setPen(QPen(Qt.gray))
            text = text + "\n(deactivated)"
        fm = QFontMetricsF(font)
        text = self.getAdjustedText(self.text)
        h = fm.ascent()
        pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, ModelerGraphicItem.BOX_HEIGHT / 2.0 - h + 1)
        painter.drawText(pt, text)
        painter.setPen(QPen(Qt.black))
        if isinstance(self.element, QgsProcessingModelChildAlgorithm):
            h = -(fm.height() * 1.2)
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'In')
            i = 1
            if not self.element.parametersCollapsed():
                for param in [p for p in self.element.algorithm().parameterDefinitions() if not p.isDestination()]:
                    if not param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                        text = self.getAdjustedText(param.description())
                        h = -(fm.height() * 1.2) * (i + 1)
                        h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
                        pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 33, h)
                        painter.drawText(pt, text)
                        i += 1
            h = fm.height() * 1.1
            h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
            pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'Out')
            if not self.element.outputsCollapsed():
                for i, out in enumerate(self.element.algorithm().outputDefinitions()):
                    text = self.getAdjustedText(out.description())
                    h = fm.height() * 1.2 * (i + 2)
                    h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
                    pt = QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 33, h)
                    painter.drawText(pt, text)
        if self.pixmap:
            painter.drawPixmap(-(ModelerGraphicItem.BOX_WIDTH / 2.0) + 3, -8,
                               self.pixmap)
        elif self.picture:
            painter.drawPicture(-(ModelerGraphicItem.BOX_WIDTH / 2.0) + 3, -8,
                                self.picture)

    def getLinkPointForParameter(self, paramIndex):
        offsetX = 25
        if isinstance(self.element, QgsProcessingModelChildAlgorithm) and self.element.parametersCollapsed():
            paramIndex = -1
            offsetX = 17
        if isinstance(self.element, QgsProcessingModelParameter):
            paramIndex = -1
            offsetX = 0
        font = QFont('Verdana', 8)
        font.setPixelSize(12)
        fm = QFontMetricsF(font)
        if isinstance(self.element, QgsProcessingModelChildAlgorithm):
            h = -(fm.height() * 1.2) * (paramIndex + 2) - fm.height() / 2.0 + 8
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0
        else:
            h = 0
        return QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + offsetX, h)

    def getLinkPointForOutput(self, outputIndex):
        if isinstance(self.element, QgsProcessingModelChildAlgorithm) and self.element.algorithm().outputDefinitions():
            outputIndex = (outputIndex if not self.element.outputsCollapsed() else -1)
            text = self.getAdjustedText(self.element.algorithm().outputDefinitions()[outputIndex].description())
            font = QFont('Verdana', 8)
            font.setPixelSize(12)
            fm = QFontMetricsF(font)
            w = fm.width(text)
            h = fm.height() * 1.2 * (outputIndex + 1) + fm.height() / 2.0
            y = h + ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            x = (-ModelerGraphicItem.BOX_WIDTH / 2 + 33 + w + 5
                 if not self.element.outputsCollapsed()
                 else 10)
            return QPointF(x, y)
        else:
            return QPointF(0, 0)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemPositionHasChanged:
            for arrow in self.arrows:
                arrow.updatePath()
            self.element.setPosition(self.pos())

            # also need to update the model's stored component's position
            if isinstance(self.element, QgsProcessingModelChildAlgorithm):
                self.model.childAlgorithm(self.element.childId()).setPosition(self.pos())
            elif isinstance(self.element, QgsProcessingModelParameter):
                self.model.parameterComponent(self.element.parameterName()).setPosition(self.pos())
            elif isinstance(self.element, QgsProcessingModelOutput):
                self.model.childAlgorithm(self.element.childId()).modelOutput(self.element.name()).setPosition(self.pos())

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
