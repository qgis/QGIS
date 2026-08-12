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

from qgis.gui import (
    QgsGui,
    QgsProcessingGui,
)

DIALOG_STANDARD = QgsProcessingGui.WidgetType.Standard
DIALOG_MODELER = QgsProcessingGui.WidgetType.Modeler

pluginPath = os.path.split(os.path.dirname(__file__))[0]

dialogTypes = {
    "AlgorithmWidget": DIALOG_STANDARD,
    "ModelerParametersDialog": DIALOG_MODELER,
}


class WidgetWrapperFactory:
    """
    Factory for parameter widget wrappers
    """

    @staticmethod
    def create_wrapper(param, dialog, row=0, col=0):
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
