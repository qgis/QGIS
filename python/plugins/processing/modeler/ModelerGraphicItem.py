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

from qgis.PyQt.QtCore import Qt, QPointF
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProject)
from qgis.gui import (
    QgsProcessingParameterDefinitionDialog,
    QgsProcessingParameterWidgetContext,
    QgsModelParameterGraphicItem,
    QgsModelChildAlgorithmGraphicItem,
    QgsModelOutputGraphicItem
)
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.tools.dataobjects import createContext
from qgis.utils import iface


class ModelerInputGraphicItem(QgsModelParameterGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

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
            self.changed.emit()


class ModelerChildAlgorithmGraphicItem(QgsModelChildAlgorithmGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

    def editComponent(self):
        elemAlg = self.component().algorithm()
        dlg = ModelerParametersDialog(elemAlg, self.model(), self.component().childId(),
                                      self.component().configuration())
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            alg.setChildId(self.component().childId())
            self.updateAlgorithm(alg)
            self.requestModelRepaint.emit()
            self.changed.emit()

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


class ModelerOutputGraphicItem(QgsModelOutputGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

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
            self.changed.emit()
