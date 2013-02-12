# -*- coding: utf-8 -*-

"""
***************************************************************************
    polygonize.py
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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtGui, QtCore
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
import os
from sextante.gdal.GdalUtils import GdalUtils
from sextante.parameters.ParameterString import ParameterString
from sextante.outputs.OutputVector import OutputVector
from sextante.core.SextanteUtils import SextanteUtils

class polygonize(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/polygonize.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Polygonize"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterRaster(polygonize.INPUT, "Input layer", False))
        self.addParameter(ParameterString(polygonize.FIELD, "Output field name", "DN"))
        self.addOutput(OutputVector(polygonize.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        if SextanteUtils.isWindows():
            commands = ["cmd.exe", "/C ", "gdal_polygonize.bat"]
        else:
            commands = ["gdal_polygonize.py"]
        commands.append(self.getParameterValue(polygonize.INPUT))
        commands.append('-f')
        commands.append('"ESRI Shapefile"')
        output = self.getOutputValue(polygonize.OUTPUT)
        commands.append(output)
        commands.append(QtCore.QFileInfo(output).baseName())
        commands.append(self.getParameterValue(polygonize.FIELD))

        GdalUtils.runGdal(commands, progress)
