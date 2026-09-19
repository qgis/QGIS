"""
***************************************************************************
    ParametersPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from qgis.core import (
    Qgis,
    QgsProcessingModelAlgorithm,
    QgsProcessingOutputLayerDefinition,
    QgsProcessingParameterDefinition,
    QgsProject,
)
from qgis.gui import (
    QgsAbstractProcessingParameterWidgetWrapper,
    QgsGui,
    QgsProcessingContextGenerator,
    QgsProcessingGui,
    QgsProcessingHiddenWidgetWrapper,
    QgsProcessingParametersGenerator,
    QgsProcessingParametersWidget,
)
from qgis.PyQt.QtWidgets import QLabel, QVBoxLayout, QWidget

from processing.core.exceptions import InvalidOutputExtension, InvalidParameterValue
from processing.tools.dataobjects import createContext


class ParametersPanel(QgsProcessingParametersWidget):
    def __init__(
        self, parent, alg, in_place=False, active_layer=None, message_bar=None
    ):
        super().__init__(alg, in_place, active_layer, message_bar, parent)
