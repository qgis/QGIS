"""
***************************************************************************
    ModelerScene.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from qgis.gui import QgsModelGraphicsScene
from processing.modeler.ModelerGraphicItem import (
    ModelerInputGraphicItem,
    ModelerOutputGraphicItem,
    ModelerChildAlgorithmGraphicItem,
)


class ModelerScene(QgsModelGraphicsScene):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, parent=None):
        super().__init__(parent)

    def createParameterGraphicItem(self, model, param):
        return ModelerInputGraphicItem(param.clone(), model)

    def createChildAlgGraphicItem(self, model, child):
        return ModelerChildAlgorithmGraphicItem(child.clone(), model)

    def createOutputGraphicItem(self, model, output):
        return ModelerOutputGraphicItem(output.clone(), model)
