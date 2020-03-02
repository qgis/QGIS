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

import os

from qgis.PyQt.QtCore import Qt, QPointF, QRectF, pyqtSignal
from qgis.PyQt.QtGui import QFont, QFontMetricsF, QPen, QBrush, QColor, QPicture, QPainter, QPalette
from qgis.PyQt.QtWidgets import QApplication, QGraphicsItem, QMessageBox, QMenu
from qgis.PyQt.QtSvg import QSvgRenderer
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingModelParameter,
                       QgsProcessingModelOutput,
                       QgsProcessingModelChildAlgorithm,
                       QgsProject)
from qgis.gui import (
    QgsProcessingParameterDefinitionDialog,
    QgsProcessingParameterWidgetContext,
    QgsModelDesignerFoldButtonGraphicItem,
    QgsModelComponentGraphicItem
)
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.tools.dataobjects import createContext
from qgis.utils import iface

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerGraphicItem(QgsModelComponentGraphicItem):

    def __init__(self, element, model):
        super().__init__(element, model, None)
        self.pixmap = None
        self.picture = None

    def boundingRect(self):
        fm = QFontMetricsF(self.font())
        unfolded = isinstance(self.component(),
                              QgsProcessingModelChildAlgorithm) and not self.component().parametersCollapsed()
        numParams = len([a for a in self.component().algorithm().parameterDefinitions() if
                         not a.isDestination()]) if unfolded else 0
        unfolded = isinstance(self.component(),
                              QgsProcessingModelChildAlgorithm) and not self.component().outputsCollapsed()
        numOutputs = len(self.component().algorithm().outputDefinitions()) if unfolded else 0

        hUp = fm.height() * 1.2 * (numParams + 2)
        hDown = fm.height() * 1.2 * (numOutputs + 2)
        rect = QRectF(-(self.component().size().width() + 2) / 2,
                      -(self.component().size().height() + 2) / 2 - hUp,
                      self.component().size().width() + 2,
                      self.component().size().height() + hDown + hUp)
        return rect

    def paint(self, painter, option, widget=None):
        rect = self.itemRect()

        if isinstance(self.component(), QgsProcessingModelParameter):
            color = QColor(238, 242, 131)
            stroke = QColor(234, 226, 118)
            selected = QColor(116, 113, 68)
        elif isinstance(self.component(), QgsProcessingModelChildAlgorithm):
            color = QColor(255, 255, 255)
            stroke = Qt.gray
            selected = QColor(50, 50, 50)
        else:
            color = QColor(172, 196, 114)
            stroke = QColor(90, 140, 90)
            selected = QColor(42, 65, 42)
        if self.state() == QgsModelComponentGraphicItem.Selected:
            stroke = selected
            color = color.darker(110)
        if self.state() == QgsModelComponentGraphicItem.Hover:
            color = color.darker(105)
        painter.setPen(QPen(stroke, 0))  # 0 width "cosmetic" pen
        painter.setBrush(QBrush(color, Qt.SolidPattern))
        painter.drawRect(rect)
        painter.setFont(self.font())
        painter.setPen(QPen(Qt.black))
        text = self.truncatedTextForItem(self.label())
        if isinstance(self.component(), QgsProcessingModelChildAlgorithm) and not self.component().isActive():
            painter.setPen(QPen(Qt.gray))
            text = text + "\n(deactivated)"
        fm = QFontMetricsF(self.font())
        text = self.truncatedTextForItem(self.label())
        h = fm.ascent()
        pt = QPointF(-self.component().size().width() / 2 + 25, self.component().size().height() / 2.0 - h + 1)
        painter.drawText(pt, text)
        painter.setPen(QPen(QApplication.palette().color(QPalette.WindowText)))
        if isinstance(self.component(), QgsProcessingModelChildAlgorithm):
            h = -(fm.height() * 1.2)
            h = h - self.component().size().height() / 2.0 + 5
            pt = QPointF(-self.component().size().width() / 2 + 25, h)
            painter.drawText(pt, 'In')
            i = 1
            if not self.component().parametersCollapsed():
                for param in [p for p in self.component().algorithm().parameterDefinitions() if not p.isDestination()]:
                    if not param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                        text = self.truncatedTextForItem(param.description())
                        h = -(fm.height() * 1.2) * (i + 1)
                        h = h - self.component().size().height() / 2.0 + 5
                        pt = QPointF(-self.component().size().width() / 2 + 33, h)
                        painter.drawText(pt, text)
                        i += 1
            h = fm.height() * 1.1
            h = h + self.component().size().height() / 2.0
            pt = QPointF(-self.component().size().width() / 2 + 25, h)
            painter.drawText(pt, 'Out')
            if not self.component().outputsCollapsed():
                for i, out in enumerate(self.component().algorithm().outputDefinitions()):
                    text = self.truncatedTextForItem(out.description())
                    h = fm.height() * 1.2 * (i + 2)
                    h = h + self.component().size().height() / 2.0
                    pt = QPointF(-self.component().size().width() / 2 + 33, h)
                    painter.drawText(pt, text)
        if self.pixmap:
            painter.drawPixmap(-(self.component().size().width() / 2.0) + 3, -8,
                               self.pixmap)
        elif self.picture:
            painter.drawPicture(-(self.component().size().width() / 2.0) + 3, -8,
                                self.picture)

    def getLinkPointForParameter(self, paramIndex):
        offsetX = 25
        if isinstance(self.component(), QgsProcessingModelChildAlgorithm) and self.component().parametersCollapsed():
            paramIndex = -1
            offsetX = 17
        if isinstance(self.component(), QgsProcessingModelParameter):
            paramIndex = -1
            offsetX = 0
        fm = QFontMetricsF(self.font())
        if isinstance(self.component(), QgsProcessingModelChildAlgorithm):
            h = -(fm.height() * 1.2) * (paramIndex + 2) - fm.height() / 2.0 + 8
            h = h - self.component().size().height() / 2.0
        else:
            h = 0
        return QPointF(-self.component().size().width() / 2 + offsetX, h)

    def getLinkPointForOutput(self, outputIndex):
        if isinstance(self.component(),
                      QgsProcessingModelChildAlgorithm) and self.component().algorithm().outputDefinitions():
            outputIndex = (outputIndex if not self.component().outputsCollapsed() else -1)
            text = self.truncatedTextForItem(self.component().algorithm().outputDefinitions()[outputIndex].description())
            fm = QFontMetricsF(self.font())
            w = fm.width(text)
            h = fm.height() * 1.2 * (outputIndex + 1) + fm.height() / 2.0
            y = h + self.component().size().height() / 2.0 + 5
            x = (-self.component().size().width() / 2 + 33 + w + 5
                 if not self.component().outputsCollapsed()
                 else 10)
            return QPointF(x, y)
        else:
            return QPointF(0, 0)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemPositionHasChanged:
            self.updateArrowPaths.emit()
            self.component().setPosition(self.pos())

            # also need to update the model's stored component's position
            if isinstance(self.component(), QgsProcessingModelChildAlgorithm):
                self.model().childAlgorithm(self.component().childId()).setPosition(self.pos())
            elif isinstance(self.component(), QgsProcessingModelParameter):
                self.model().parameterComponent(self.component().parameterName()).setPosition(self.pos())
            elif isinstance(self.component(), QgsProcessingModelOutput):
                self.model().childAlgorithm(self.component().childId()).modelOutput(
                    self.component().name()).setPosition(self.pos())
        elif change == QGraphicsItem.ItemSelectedChange:
            self.repaintArrows.emit()

        return super().itemChange(change, value)


class ModelerInputGraphicItem(ModelerGraphicItem):

    def __init__(self, element, model):
        super().__init__(element, model)

        svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'input.svg'))
        self.picture = QPicture()
        painter = QPainter(self.picture)
        svg.render(painter)
        painter.end()
        paramDef = self.model().parameterDefinition(element.parameterName())
        if paramDef:
            self.setLabel(paramDef.description())
        else:
            self.setLabel('Error ({})'.format(element.parameterName()))

    def create_widget_context(self):
        """
        Returns a new widget context for use in the model editor
        """
        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
        widget_context.setModel(self.model())
        return widget_context

    def editComponent(self):
        existing_param = self.model().parameterDefinition(self.component().parameterName())
        new_param = None
        if ModelerParameterDefinitionDialog.use_legacy_dialog(param=existing_param):
            # boo, old api
            dlg = ModelerParameterDefinitionDialog(self.model(),
                                                   param=existing_param)
            if dlg.exec_():
                new_param = dlg.param
        else:
            # yay, use new API!
            context = createContext()
            widget_context = self.create_widget_context()
            dlg = QgsProcessingParameterDefinitionDialog(type=existing_param.type(),
                                                         context=context,
                                                         widgetContext=widget_context,
                                                         definition=existing_param,
                                                         algorithm=self.model())
            if dlg.exec_():
                new_param = dlg.createParameter(existing_param.name())

        if new_param is not None:
            self.model().removeModelParameter(self.component().parameterName())
            self.component().setParameterName(new_param.name())
            self.component().setDescription(new_param.name())
            self.model().addModelParameter(new_param, self.component())
            self.setLabel(new_param.description())
            self.requestModelRepaint.emit()

    def deleteComponent(self):
        if self.model().childAlgorithmsDependOnParameter(self.component().parameterName()):
            QMessageBox.warning(None, 'Could not remove input',
                                'Algorithms depend on the selected input.\n'
                                'Remove them before trying to remove it.')
        elif self.model().otherParametersDependOnParameter(self.component().parameterName()):
            QMessageBox.warning(None, 'Could not remove input',
                                'Other inputs depend on the selected input.\n'
                                'Remove them before trying to remove it.')
        else:
            self.model().removeModelParameter(self.component().parameterName())
            self.changed.emit()
            self.requestModelRepaint.emit()

    def contextMenuEvent(self, event):
        popupmenu = QMenu()
        removeAction = popupmenu.addAction('Remove')
        removeAction.triggered.connect(self.deleteComponent)
        editAction = popupmenu.addAction('Edit')
        editAction.triggered.connect(self.editComponent)
        popupmenu.exec_(event.screenPos())


class ModelerChildAlgorithmGraphicItem(ModelerGraphicItem):

    def __init__(self, element, model):
        super().__init__(element, model)

        if element.algorithm().svgIconPath():
            svg = QSvgRenderer(element.algorithm().svgIconPath())
            size = svg.defaultSize()
            self.picture = QPicture()
            painter = QPainter(self.picture)
            painter.scale(16 / size.width(), 16 / size.width())
            svg.render(painter)
            painter.end()
            self.pixmap = None
        else:
            self.pixmap = element.algorithm().icon().pixmap(15, 15)
        self.setLabel(element.description())

        alg = element.algorithm()
        if [a for a in alg.parameterDefinitions() if not a.isDestination()]:
            pt = self.getLinkPointForParameter(-1)
            pt = QPointF(0, pt.y())
            self.inButton = QgsModelDesignerFoldButtonGraphicItem(self, self.component().parametersCollapsed(), pt)
            self.inButton.folded.connect(self.foldInput)
        if alg.outputDefinitions():
            pt = self.getLinkPointForOutput(-1)
            pt = QPointF(0, pt.y())
            self.outButton = QgsModelDesignerFoldButtonGraphicItem(self, self.component().outputsCollapsed(), pt)
            self.outButton.folded.connect(self.foldOutput)

    def foldInput(self, folded):
        self.component().setParametersCollapsed(folded)
        # also need to update the model's stored component
        self.model().childAlgorithm(self.component().childId()).setParametersCollapsed(folded)
        self.prepareGeometryChange()
        if self.component().algorithm().outputDefinitions():
            pt = self.getLinkPointForOutput(-1)
            pt = QPointF(0, pt.y())
            self.outButton.position = pt

        self.updateArrowPaths.emit()
        self.update()

    def foldOutput(self, folded):
        self.component().setOutputsCollapsed(folded)
        # also need to update the model's stored component
        self.model().childAlgorithm(self.component().childId()).setOutputsCollapsed(folded)
        self.prepareGeometryChange()
        self.updateArrowPaths.emit()
        self.update()

    def editComponent(self):
        elemAlg = self.component().algorithm()
        dlg = ModelerParametersDialog(elemAlg, self.model(), self.component().childId(),
                                      self.component().configuration())
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            alg.setChildId(self.component().childId())
            self.updateAlgorithm(alg)
            self.requestModelRepaint.emit()

    def updateAlgorithm(self, alg):
        existing_child = self.model().childAlgorithm(alg.childId())
        alg.setPosition(existing_child.position())
        alg.setParametersCollapsed(existing_child.parametersCollapsed())
        alg.setOutputsCollapsed(existing_child.outputsCollapsed())
        for i, out in enumerate(alg.modelOutputs().keys()):
            alg.modelOutput(out).setPosition(alg.modelOutput(out).position() or
                                             alg.position() + QPointF(
                self.component().size().width(),
                (i + 1.5) * self.component().size().height()))
        self.model().setChildAlgorithm(alg)

    def deleteComponent(self):
        if not self.model().removeChildAlgorithm(self.component().childId()):
            QMessageBox.warning(None, 'Could not remove element',
                                'Other elements depend on the selected one.\n'
                                'Remove them before trying to remove it.')
        else:
            self.changed.emit()
            self.requestModelRepaint.emit()

    def contextMenuEvent(self, event):
        popupmenu = QMenu()
        removeAction = popupmenu.addAction('Remove')
        removeAction.triggered.connect(self.deleteComponent)
        editAction = popupmenu.addAction('Edit')
        editAction.triggered.connect(self.editComponent)

        if not self.component().isActive():
            removeAction = popupmenu.addAction('Activate')
            removeAction.triggered.connect(self.activateAlgorithm)
        else:
            deactivateAction = popupmenu.addAction('Deactivate')
            deactivateAction.triggered.connect(self.deactivateAlgorithm)

        popupmenu.exec_(event.screenPos())

    def deactivateAlgorithm(self):
        self.model().deactivateChildAlgorithm(self.component().childId())
        self.requestModelRepaint.emit()

    def activateAlgorithm(self):
        if self.model().activateChildAlgorithm(self.component().childId()):
            self.requestModelRepaint.emit()
        else:
            QMessageBox.warning(None, 'Could not activate Algorithm',
                                'The selected algorithm depends on other currently non-active algorithms.\n'
                                'Activate them them before trying to activate it.')


class ModelerOutputGraphicItem(ModelerGraphicItem):

    def __init__(self, element, model):
        super().__init__(element, model)

        # Output name
        svg = QSvgRenderer(os.path.join(pluginPath, 'images', 'output.svg'))
        self.picture = QPicture()
        painter = QPainter(self.picture)
        svg.render(painter)
        painter.end()
        self.setLabel(element.name())

    def editComponent(self):
        child_alg = self.model().childAlgorithm(self.component().childId())
        param_name = '{}:{}'.format(self.component().childId(), self.component().name())
        dlg = ModelerParameterDefinitionDialog(self.model(),
                                               param=self.model().parameterDefinition(param_name))
        if dlg.exec_() and dlg.param is not None:
            model_output = child_alg.modelOutput(self.component().name())
            model_output.setDescription(dlg.param.description())
            model_output.setDefaultValue(dlg.param.defaultValue())
            model_output.setMandatory(not (dlg.param.flags() & QgsProcessingParameterDefinition.FlagOptional))
            self.model().updateDestinationParameters()

    def deleteComponent(self):
        self.model().childAlgorithm(self.component().childId()).removeModelOutput(self.component().name())
        self.model().updateDestinationParameters()
        self.changed.emit()
        self.requestModelRepaint.emit()

    def contextMenuEvent(self, event):
        return
