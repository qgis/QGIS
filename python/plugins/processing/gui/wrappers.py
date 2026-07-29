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

from qgis.core import (
    QgsProcessingParameterDefinition,
    QgsProcessingParameterExtent,
    QgsProcessingParameterPoint,
    QgsSettings,
)
from qgis.gui import (
    QgsAbstractProcessingParameterWidgetWrapper,
    QgsGui,
    QgsProcessingGui,
)
from qgis.PyQt.QtWidgets import (
    QFileDialog,
    QLabel,
)
from qgis.utils import iface

from processing.core.exceptions import InvalidParameterValue
from processing.core.ProcessingConfig import ProcessingConfig

DIALOG_STANDARD = QgsProcessingGui.WidgetType.Standard
DIALOG_BATCH = QgsProcessingGui.WidgetType.Batch
DIALOG_MODELER = QgsProcessingGui.WidgetType.Modeler

pluginPath = os.path.split(os.path.dirname(__file__))[0]

dialogTypes = {
    "AlgorithmWidget": DIALOG_STANDARD,
    "ModelerParametersDialog": DIALOG_MODELER,
    "BatchAlgorithmDialog": DIALOG_BATCH,
}


def getExtendedLayerName(layer):
    authid = layer.crs().authid()
    if (
        ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF)
        and authid is not None
    ):
        return f"{layer.name()} [{authid}]"
    else:
        return layer.name()


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

    def comboValue(self, validator=None, combobox=None):
        if combobox is None:
            combobox = self.widget
        idx = combobox.findText(combobox.currentText())
        if idx < 0:
            v = combobox.currentText().strip()
            if validator is not None and not validator(v):
                raise InvalidParameterValue(self.param, self.widget)
            return v
        if combobox.currentData() == self.NOT_SET_OPTION:
            return None
        elif combobox.currentData() is not None:
            return combobox.currentData()
        else:
            return combobox.currentText()

    def createWidget(self, **kwargs):
        pass

    def createLabel(self):
        if self.dialogType == DIALOG_BATCH:
            return None
        desc = self.parameterDefinition().description()
        if isinstance(self.parameterDefinition(), QgsProcessingParameterExtent):
            desc += self.tr(" (xmin, xmax, ymin, ymax)")
        if isinstance(self.parameterDefinition(), QgsProcessingParameterPoint):
            desc += self.tr(" (x, y)")
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

    def setComboValue(self, value, combobox=None):
        if combobox is None:
            combobox = self.widget
        if isinstance(value, list):
            if value:
                value = value[0]
            else:
                value = None
        values = [combobox.itemData(i) for i in range(combobox.count())]
        try:
            idx = values.index(value)
            combobox.setCurrentIndex(idx)
            return
        except ValueError:
            pass
        if combobox.isEditable():
            if value is not None:
                combobox.setEditText(str(value))
        else:
            combobox.setCurrentIndex(0)

    def refresh(self):
        pass

    def getFileName(self, initial_value=""):
        """Shows a file open dialog"""
        settings = QgsSettings()
        if os.path.isdir(initial_value):
            path = initial_value
        elif os.path.isdir(os.path.dirname(initial_value)):
            path = os.path.dirname(initial_value)
        elif settings.contains("/Processing/LastInputPath"):
            path = str(settings.value("/Processing/LastInputPath"))
        else:
            path = ""

        # TODO: should use selectedFilter argument for default file format
        filename, selected_filter = QFileDialog.getOpenFileName(
            self.widget,
            self.tr("Select File"),
            path,
            self.parameterDefinition().createFileFilter(),
        )
        if filename:
            settings.setValue(
                "/Processing/LastInputPath", os.path.dirname(str(filename))
            )
        return filename, selected_filter


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
        else:
            # try from c++ registry first
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
            if wrapper is not None:
                wrapper.setDialog(dialog)
                return wrapper

            # fallback to Python registry
            return WidgetWrapperFactory.create_wrapper_from_class(
                param, dialog, row, col
            )

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

    @staticmethod
    def create_wrapper_from_class(param, dialog, row=0, col=0):
        assert False, param.type()
