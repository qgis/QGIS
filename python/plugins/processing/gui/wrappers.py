"""
***************************************************************************
    wrappers.py - Standard parameters widget wrappers
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Arnaud Morvan, Victor Olaya
    Email                : arnaud dot morvan at camptocamp dot com
                           volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Arnaud Morvan"
__date__ = "May 2016"
__copyright__ = "(C) 2016, Arnaud Morvan"

import os
from copy import deepcopy
from inspect import isclass

from qgis.core import QgsProcessingParameterDefinition
from qgis.gui import (
    QgsAbstractProcessingParameterWidgetWrapper,
    QgsGui,
    QgsProcessingGui,
)
from qgis.PyQt.QtWidgets import (
    QLabel,
)
from qgis.utils import iface

DIALOG_STANDARD = QgsProcessingGui.WidgetType.Standard
DIALOG_BATCH = QgsProcessingGui.WidgetType.Batch
DIALOG_MODELER = QgsProcessingGui.WidgetType.Modeler

pluginPath = os.path.split(os.path.dirname(__file__))[0]

dialogTypes = {
    "AlgorithmWidget": DIALOG_STANDARD,
    "ModelerParametersDialog": DIALOG_MODELER,
    "BatchAlgorithmDialog": DIALOG_BATCH,
}


class WidgetWrapper(QgsAbstractProcessingParameterWidgetWrapper):
    NOT_SET_OPTION = "~~~~!!!!NOT SET!!!!~~~~~~~"

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        self.dialogType = dialogTypes.get(
            dialog.__class__.__name__, QgsProcessingGui.WidgetType.Standard
        )
        super().__init__(param, self.dialogType)

        self.dialog = dialog
        self.row = row
        self.col = col

        self.widget = self.createWidget(**kwargs)
        self.label = self.createLabel()
        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())

    def createWidget(self, **kwargs):
        pass

    def createLabel(self):
        if self.dialogType == DIALOG_BATCH:
            return None
        desc = self.parameterDefinition().description()
        if (
            self.parameterDefinition().flags()
            & QgsProcessingParameterDefinition.Flag.FlagOptional
        ):
            desc += self.tr(" [optional]")

        label = QLabel(desc)
        label.setToolTip(self.parameterDefinition().name())
        return label

    def setValue(self, value):
        pass

    def value(self):
        return None

    def widgetValue(self):
        return self.value()

    def setWidgetValue(self, value, context):
        self.setValue(value)

    def refresh(self):
        pass


class WidgetWrapperFactory:
    """
    Factory for parameter widget wrappers
    """

    @staticmethod
    def create_wrapper(param, dialog, row=0, col=0):
        wrapper_metadata = param.metadata().get("widget_wrapper", None)
        # VERY messy logic here to avoid breaking 3.0 API which allowed metadata "widget_wrapper" value to be either
        # a string name of a class OR a dict.
        # TODO QGIS 5.0 -- require widget_wrapper to be a dict.
        if wrapper_metadata and (
            not isinstance(wrapper_metadata, dict)
            or wrapper_metadata.get("class", None) is not None
        ):
            return WidgetWrapperFactory.create_wrapper_from_metadata(
                param, dialog, row, col
            )

        # retrieve from c++ registry
        class_type = dialog.__class__.__name__
        if class_type == "ModelerParametersDialog":
            wrapper = QgsGui.processingGuiRegistry().createModelerParameterWidget(
                dialog.model, dialog.childId, param, dialog.context
            )
        else:
            dialog_type = dialogTypes.get(
                class_type, QgsProcessingGui.WidgetType.Standard
            )
            wrapper = QgsGui.processingGuiRegistry().createParameterWidgetWrapper(
                param, dialog_type
            )

        wrapper.setDialog(dialog)
        return wrapper

    @staticmethod
    def create_wrapper_from_metadata(param, dialog, row=0, col=0):
        wrapper = param.metadata().get("widget_wrapper", None)
        params = {}
        # wrapper metadata should be a dict with class key
        if isinstance(wrapper, dict):
            params = deepcopy(wrapper)
            wrapper = params.pop("class")
        # wrapper metadata should be a class path
        if isinstance(wrapper, str):
            tokens = wrapper.split(".")
            mod = __import__(".".join(tokens[:-1]), fromlist=[tokens[-1]])
            wrapper = getattr(mod, tokens[-1])
        # or directly a class object
        if isclass(wrapper):
            wrapper = wrapper(param, dialog, row, col, **params)
        # or a wrapper instance
        return wrapper
