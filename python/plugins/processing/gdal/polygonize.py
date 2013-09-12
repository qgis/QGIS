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

import os
from PyQt4 import QtGui, QtCore

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools.system import *

from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterString import ParameterString
from processing.outputs.OutputVector import OutputVector

from processing.gdal.GdalUtils import GdalUtils

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
        arguments = []
        arguments.append(self.getParameterValue(polygonize.INPUT))
        arguments.append('-f')
        arguments.append('"ESRI Shapefile"')
        output = self.getOutputValue(polygonize.OUTPUT)
        arguments.append(output)
        arguments.append(QtCore.QFileInfo(output).baseName())
        arguments.append(self.getParameterValue(polygonize.FIELD))

        commands = []
        if isWindows():
            commands = ["cmd.exe", "/C ", "gdal_polygonize.bat", GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ["gdal_polygonize.py", GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
