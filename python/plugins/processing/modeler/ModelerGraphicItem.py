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

from qgis.PyQt.QtCore import Qt, QPointF
from qgis.PyQt.QtGui import QFontMetricsF, QPen, QBrush, QColor, QPicture, QPainter, QPalette
from qgis.PyQt.QtWidgets import QApplication, QMessageBox, QMenu
from qgis.PyQt.QtSvg import QSvgRenderer
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingModelParameter,
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

        if self.linkPointCount(Qt.TopEdge) or self.linkPointCount(Qt.BottomEdge):
            h = -(fm.height() * 1.2)
            h = h - self.component().size().height() / 2.0 + 5
            pt = QPointF(-self.component().size().width() / 2 + 25, h)
            painter.drawText(pt, 'In')
            i = 1
            if not self.component().linksCollapsed(Qt.TopEdge):
                for idx in range(self.linkPointCount(Qt.TopEdge)):
                    text = self.linkPointText(Qt.TopEdge, idx)
                    h = -(fm.height() * 1.2) * (i + 1)
                    h = h - self.component().size().height() / 2.0 + 5
                    pt = QPointF(-self.component().size().width() / 2 + 33, h)
                    painter.drawText(pt, text)
                    i += 1

            h = fm.height() * 1.1
            h = h + self.component().size().height() / 2.0
            pt = QPointF(-self.component().size().width() / 2 + 25, h)
            painter.drawText(pt, 'Out')
            if not self.component().linksCollapsed(Qt.BottomEdge):
                for idx in range(self.linkPointCount(Qt.BottomEdge)):
                    text = self.linkPointText(Qt.BottomEdge, idx)
                    h = fm.height() * 1.2 * (idx + 2)
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
        if isinstance(self.component(), QgsProcessingModelChildAlgorithm) and self.component().linksCollapsed(
                Qt.TopEdge):
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
            self.inButton = QgsModelDesignerFoldButtonGraphicItem(self, self.component().linksCollapsed(Qt.TopEdge), pt)
            self.inButton.folded.connect(self.foldInput)
        if alg.outputDefinitions():
            pt = self.linkPoint(Qt.BottomEdge, -1)
            pt = QPointF(0, pt.y())
            self.outButton = QgsModelDesignerFoldButtonGraphicItem(self, self.component().linksCollapsed(Qt.BottomEdge),
                                                                   pt)
            self.outButton.folded.connect(self.foldOutput)

    def linkPointCount(self, edge):
        if edge == Qt.BottomEdge:
            return len(self.component().algorithm().outputDefinitions())
        elif edge == Qt.TopEdge:
            return len([p for p in self.component().algorithm().parameterDefinitions() if
                        not p.isDestination() and not p.flags() & QgsProcessingParameterDefinition.FlagHidden])

        return 0

    def linkPointText(self, edge, index):
        if edge == Qt.TopEdge:
            param = [p for p in self.component().algorithm().parameterDefinitions() if
                     not p.isDestination() and not p.flags() & QgsProcessingParameterDefinition.FlagHidden][index]
            return self.truncatedTextForItem(param.description())
        elif edge == Qt.BottomEdge:
            out = self.component().algorithm().outputDefinitions()[index]
            return self.truncatedTextForItem(out.description())

    def foldInput(self, folded):
        self.component().setLinksCollapsed(Qt.TopEdge, folded)
        # also need to update the model's stored component
        self.model().childAlgorithm(self.component().childId()).setLinksCollapsed(Qt.TopEdge, folded)
        self.prepareGeometryChange()
        if self.component().algorithm().outputDefinitions():
            pt = self.linkPoint(Qt.BottomEdge, -1)
            pt = QPointF(0, pt.y())
            self.outButton.position = pt

        self.updateArrowPaths.emit()
        self.update()

    def foldOutput(self, folded):
        self.component().setLinksCollapsed(Qt.BottomEdge, folded)
        # also need to update the model's stored component
        self.model().childAlgorithm(self.component().childId()).setLinksCollapsed(Qt.BottomEdge, folded)
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
        alg.setLinksCollapsed(Qt.TopEdge, existing_child.linksCollapsed(Qt.TopEdge))
        alg.setLinksCollapsed(Qt.BottomEdge, existing_child.linksCollapsed(Qt.BottomEdge))
        for i, out in enumerate(alg.modelOutputs().keys()):
            alg.modelOutput(out).setPosition(alg.modelOutput(out).position()
                                             or alg.position() + QPointF(
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
