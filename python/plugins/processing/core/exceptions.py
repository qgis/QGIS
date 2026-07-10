"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import QgsProcessingParameterDefinition
from qgis.PyQt.QtWidgets import QWidget


class InvalidParameterValue(Exception):
    """
    Raised when an invalid parameter value is entered in a widget
    """

    def __init__(self, param: QgsProcessingParameterDefinition, widget: QWidget):
        self.parameter: QgsProcessingParameterDefinition = param
        self.widget: QWidget = widget


class InvalidOutputExtension(Exception):
    """
    Raised when a parameter value has an invalid output file extension
    """

    def __init__(self, widget: QWidget, message: str):
        self.widget: QWidget = widget
        self.message: str = message
