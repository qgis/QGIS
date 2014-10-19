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
from PyQt4 import QtCore, QtGui
from processing.modeler.ModelerAlgorithm import ModelerParameter, Algorithm, ModelerOutput
from processing.modeler.ModelerParameterDefinitionDialog import \
        ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog


class ModelerGraphicItem(QtGui.QGraphicsItem):

    BOX_HEIGHT = 30
    BOX_WIDTH = 200

    def __init__(self, element, model):
        super(ModelerGraphicItem, self).__init__(None, None)
        self.model = model
        self.element = element
        if isinstance(element, ModelerParameter):
            icon = QtGui.QIcon(os.path.dirname(__file__)
                               + '/../images/input.png')
            self.pixmap = icon.pixmap(20, 20, state=QtGui.QIcon.On)
            self.text = element.param.description
        elif isinstance(element, ModelerOutput):
            # Output name
            icon = QtGui.QIcon(os.path.dirname(__file__)
                               + '/../images/output.png')
            self.pixmap = icon.pixmap(20, 20, state=QtGui.QIcon.On)
            self.text = element.description
        else:
            self.text = element.description
            self.pixmap = element.algorithm.getIcon().pixmap(15, 15)
        self.arrows = []
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
        self.setFlag(QtGui.QGraphicsItem.ItemSendsGeometryChanges, True)
        self.setZValue(1000)

        if not isinstance(element, ModelerOutput):
            icon = QtGui.QIcon(os.path.dirname(__file__)
                               + '/../images/edit.png')
            pt = QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH / 2
                                - FlatButtonGraphicItem.WIDTH / 2,
                                ModelerGraphicItem.BOX_HEIGHT / 2
                                - FlatButtonGraphicItem.HEIGHT / 2 + 1)
            self.editButton = FlatButtonGraphicItem(icon, pt, self.editElement)
            self.editButton.setParentItem(self)
            icon = QtGui.QIcon(os.path.dirname(__file__)
                               + '/../images/delete.png')
            pt = QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH / 2
                                - FlatButtonGraphicItem.WIDTH / 2,
                                -ModelerGraphicItem.BOX_HEIGHT / 2
                                + FlatButtonGraphicItem.HEIGHT / 2 + 1)
            self.deleteButton = FlatButtonGraphicItem(icon, pt,
                    self.removeElement)
            self.deleteButton.setParentItem(self)

        if isinstance(element, Algorithm):
            alg = element.algorithm
            if alg.parameters:
                pt = self.getLinkPointForParameter(-1)
                pt = QtCore.QPointF(0, pt.y() + 2)
                self.inButton = FoldButtonGraphicItem(pt, self.foldInput, self.element.paramsFolded)
                self.inButton.setParentItem(self)
            if alg.outputs:
                pt = self.getLinkPointForOutput(-1)
                pt = QtCore.QPointF(0, pt.y() + 2)
                self.outButton = FoldButtonGraphicItem(pt, self.foldOutput, self.element.outputsFolded)
                self.outButton.setParentItem(self)

    def foldInput(self, folded):
        self.element.paramsFolded = folded
        self.prepareGeometryChange()
        if self.element.algorithm.outputs:
            pt = self.getLinkPointForOutput(-1)
            pt = QtCore.QPointF(0, pt.y())
            self.outButton.position = pt
        self.update()

    def foldOutput(self, folded):
        self.element.outputsFolded = folded
        self.prepareGeometryChange()
        self.update()

    def addArrow(self, arrow):
        self.arrows.append(arrow)

    def boundingRect(self):
        font = QtGui.QFont('Verdana', 8)
        fm = QtGui.QFontMetricsF(font)
        unfolded = isinstance(self.element, Algorithm) and not self.element.paramsFolded
        numParams = len(self.element.algorithm.parameters) if unfolded else 0
        unfolded = isinstance(self.element, Algorithm) and not self.element.outputsFolded
        numOutputs = len(self.element.algorithm.outputs) if unfolded else 0

        hUp = fm.height() * 1.2 * (numParams + 2)
        hDown = fm.height() * 1.2 * (numOutputs + 2)
        rect = QtCore.QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                             -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp,
                             ModelerGraphicItem.BOX_WIDTH + 2,
                             ModelerGraphicItem.BOX_HEIGHT + hDown + hUp)
        return rect

    def mouseDoubleClickEvent(self, event):
        pass
        #self.editElement()

    def contextMenuEvent(self, event):
        if isinstance(self.element, ModelerOutput):
            return
        popupmenu = QtGui.QMenu()
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
            QtGui.QMessageBox.warning(None, 'Could not activate Algorithm',
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
                QtGui.QMessageBox.warning(None, 'Could not remove element',
                        'Other elements depend on the selected one.\n'
                        'Remove them before trying to remove it.')
            else:
                self.model.updateModelerView()
        elif isinstance(self.element, Algorithm):
            if not self.model.removeAlgorithm(self.element.name):
                QtGui.QMessageBox.warning(None, 'Could not remove element',
                        'Other elements depend on the selected one.\n'
                        'Remove them before trying to remove it.')
            else:
                self.model.updateModelerView()

    def getAdjustedText(self, text):
        font = QtGui.QFont('Verdana', 8)
        fm = QtGui.QFontMetricsF(font)
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
        rect = QtCore.QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2) / 2.0,
                             -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2.0,
                             ModelerGraphicItem.BOX_WIDTH + 2,
                             ModelerGraphicItem.BOX_HEIGHT + 2)
        painter.setPen(QtGui.QPen(QtCore.Qt.gray, 1))
        color = QtGui.QColor(125, 232, 232)
        if isinstance(self.element, ModelerParameter):
            color = QtGui.QColor(179, 179, 255)
        elif isinstance(self.element, Algorithm):
            color = QtCore.Qt.white
        painter.setBrush(QtGui.QBrush(color, QtCore.Qt.SolidPattern))
        painter.drawRect(rect)
        font = QtGui.QFont('Verdana', 8)
        painter.setFont(font)
        painter.setPen(QtGui.QPen(QtCore.Qt.black))
        text = self.getAdjustedText(self.text)
        if isinstance(self.element, Algorithm) and not self.element.active:
            painter.setPen(QtGui.QPen(QtCore.Qt.gray))
            text = text + "\n(deactivated)"
        elif self.isSelected():
            painter.setPen(QtGui.QPen(QtCore.Qt.blue))
        fm = QtGui.QFontMetricsF(font)
        text = self.getAdjustedText(self.text)
        h = fm.height()
        pt = QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h / 2.0)
        painter.drawText(pt, text)
        painter.setPen(QtGui.QPen(QtCore.Qt.black))
        if isinstance(self.element, Algorithm):
            h = -(fm.height() * 1.2)
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            pt = QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'In')
            i = 1
            if not self.element.paramsFolded:
                for param in self.element.algorithm.parameters:
                    if not param.hidden:
                        text = self.getAdjustedText(param.description)
                        h = -(fm.height() * 1.2) * (i + 1)
                        h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
                        pt = QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2
                                + 33, h)
                        painter.drawText(pt, text)
                        i += 1
            h = fm.height() * 1.2
            h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
            pt = QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + 25, h)
            painter.drawText(pt, 'Out')
            if not self.element.outputsFolded:
                for i, out in enumerate(self.element.algorithm.outputs):
                        text = self.getAdjustedText(out.description)
                        h = fm.height() * 1.2 * (i + 2)
                        h = h + ModelerGraphicItem.BOX_HEIGHT / 2.0
                        pt = QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2
                                + 33, h)
                        painter.drawText(pt, text)
        if self.pixmap:
            painter.drawPixmap(-(ModelerGraphicItem.BOX_WIDTH / 2.0) + 3, -8,
                               self.pixmap)

    def getLinkPointForParameter(self, paramIndex):
        offsetX = 25
        if isinstance(self.element, Algorithm) and self.element.paramsFolded:
            paramIndex = -1
            offsetX = 17
        font = QtGui.QFont('Verdana', 8)
        fm = QtGui.QFontMetricsF(font)
        if isinstance(self.element, Algorithm):
            h = -(fm.height() * 1.2) * (paramIndex + 2) - fm.height() / 2.0 + 8
            h = h - ModelerGraphicItem.BOX_HEIGHT / 2.0
        else:
            h = 0
        return QtCore.QPointF(-ModelerGraphicItem.BOX_WIDTH / 2 + offsetX, h)


    def getLinkPointForOutput(self, outputIndex):
        if isinstance(self.element, Algorithm):
            outputIndex = (outputIndex if not self.element.outputsFolded else -1)
            text = self.getAdjustedText(
                    self.element.algorithm.outputs[outputIndex].description)
            font = QtGui.QFont('Verdana', 8)
            fm = QtGui.QFontMetricsF(font)
            w = fm.width(text)
            h = fm.height() * 1.2 * (outputIndex + 1) + fm.height() / 2.0
            y = h + ModelerGraphicItem.BOX_HEIGHT / 2.0 + 5
            x = (-ModelerGraphicItem.BOX_WIDTH / 2 + 33 + w
                 + 5 if not self.element.outputsFolded else 10)
            return QtCore.QPointF(x, y)
        else:
            return QtCore.QPointF(0, 0)

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemPositionHasChanged:
            for arrow in self.arrows:
                arrow.updatePosition()
            self.element.pos = self.pos()

        return value

    def polygon(self):
        font = QtGui.QFont('Verdana', 8)
        fm = QtGui.QFontMetricsF(font)
        hUp = fm.height() * 1.2 * (len(self.element.parameters) + 2)
        hDown = fm.height() * 1.2 * (len(self.element.outputs) + 2)
        pol = QtGui.QPolygonF([
                QtCore.QPointF(
                        -(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                        -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp),
                QtCore.QPointF(
                        -(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                        (ModelerGraphicItem.BOX_HEIGHT + 2) / 2 + hDown),
                QtCore.QPointF(
                        (ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                        (ModelerGraphicItem.BOX_HEIGHT + 2) / 2 + hDown),
                QtCore.QPointF(
                        (ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                        -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp),
                QtCore.QPointF(
                        -(ModelerGraphicItem.BOX_WIDTH + 2) / 2,
                        -(ModelerGraphicItem.BOX_HEIGHT + 2) / 2 - hUp)])
        return pol


class FlatButtonGraphicItem(QtGui.QGraphicsItem):

    WIDTH = 16
    HEIGHT = 16

    def __init__(self, icon, position, action):
        super(FlatButtonGraphicItem, self).__init__(None, None)
        self.setAcceptHoverEvents(True)
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable, False)
        self.pixmap = icon.pixmap(self.WIDTH, self.HEIGHT,
                                  state=QtGui.QIcon.On)
        self.position = position
        self.isIn = False
        self.action = action

    def mousePressEvent(self, event):
        self.action()

    def paint(self, painter, option, widget=None):
        pt = QtCore.QPointF(-self.WIDTH / 2, -self.HEIGHT / 2) + self.position
        rect = QtCore.QRectF(pt.x(), pt.y(), self.WIDTH, self.HEIGHT)
        if self.isIn:
            painter.setPen(QtGui.QPen(QtCore.Qt.transparent, 1))
            painter.setBrush(QtGui.QBrush(QtCore.Qt.lightGray,
                             QtCore.Qt.SolidPattern))
        else:
            painter.setPen(QtGui.QPen(QtCore.Qt.transparent, 1))
            painter.setBrush(QtGui.QBrush(QtCore.Qt.transparent,
                             QtCore.Qt.SolidPattern))
        painter.drawRect(rect)
        painter.drawPixmap(pt.x(), pt.y(), self.pixmap)

    def boundingRect(self):
        rect = QtCore.QRectF(self.position.x() - self.WIDTH / 2,
                             self.position.y() - self.HEIGHT / 2, self.WIDTH,
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

    icons = {True: QtGui.QIcon(os.path.dirname(__file__)
             + '/../images/plus.png'),
             False: QtGui.QIcon(os.path.dirname(__file__)
             + '/../images/minus.png')}

    def __init__(self, position, action, folded):
        self.folded = folded
        icon = self.icons[self.folded]
        super(FoldButtonGraphicItem, self).__init__(icon, position, action)

    def mousePressEvent(self, event):
        self.folded = not self.folded
        icon = self.icons[self.folded]
        self.pixmap = icon.pixmap(self.WIDTH, self.HEIGHT,
                                  state=QtGui.QIcon.On)
        self.action(self.folded)
