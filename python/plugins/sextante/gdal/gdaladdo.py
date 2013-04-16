# -*- coding: utf-8 -*-

"""
***************************************************************************
    translate.py
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
from sextante.parameters.ParameterString import ParameterString

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4 import QtGui

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster

from sextante.gdal.GdalUtils import GdalUtils

class gdaladdo(GeoAlgorithm):

    INPUT = "INPUT"
    LEVELS = "LEVELS"
    OUTPUT = "OUTPUT"


    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/raster-overview.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Build pyramids (overviews)"
        self.group = "[GDAL] Miscellaneous"
        self.addParameter(ParameterRaster(gdaladdo.INPUT, "Input layer", False))
        self.addParameter(ParameterString(gdaladdo.LEVELS, "Overview levels", "2 4 8 16"))
        self.addOutput(OutputRaster(gdaladdo.OUTPUT, "Output layer", True))

    def processAlgorithm(self, progress):
        arguments = []
        inFile = self.getParameterValue(gdaladdo.INPUT)
        arguments.append(inFile)
        arguments.extend(self.getParameterValue(gdaladdo.LEVELS).split(" "))
        self.setOutputValue(gdaladdo.OUTPUT, inFile)

        GdalUtils.runGdal(["gdaladdo", GdalUtils.escapeAndJoin(arguments)], progress)
