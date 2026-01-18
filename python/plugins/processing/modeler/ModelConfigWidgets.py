"""
***************************************************************************
    ModelConfigWidgets.py
    ---------------------
    Date                 : January 2026
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import (
    QgsProcessingModelChildAlgorithm,
    QgsProcessingModelParameter,
    QgsProcessingContext,
)
from qgis.gui import (
    QgsProcessingModelConfigWidgetFactory,
    QgsProcessingParameterWidgetContext,
    QgsProcessingParameterDefinitionPanelWidget,
)
from processing.modeler.ModelerParameterDefinitionDialog import (
    ModelerParameterDefinitionDialog,
)


class ModelConfigWidgetFactory(QgsProcessingModelConfigWidgetFactory):

    def supportsComponent(self, component):
        return False

    def createWidget(
        self,
        component,
        context: QgsProcessingContext,
        widgetContext: QgsProcessingParameterWidgetContext,
    ):
        return None
