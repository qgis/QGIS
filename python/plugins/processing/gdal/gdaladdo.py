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
from processing.parameters.ParameterString import ParameterString

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4 import QtGui

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterSelection import ParameterSelection
from processing.outputs.OutputRaster import OutputRaster

from processing.gdal.GdalUtils import GdalUtils

class gdaladdo(GeoAlgorithm):

    INPUT = "INPUT"
    LEVELS = "LEVELS"
    CLEAN = "CLEAN"
    RESAMPLING_METHOD = "RESAMPLING_METHOD"
    FORMAT = "FORMAT"
    OUTPUT = "OUTPUT"

    METHODS = ["nearest", "average", "gauss", "cubic", "average_mp",
               "average_magphase", "mode"
              ]

    FORMATS = ["Internal (if possible)",
               "External (GTiff .ovr)",
               "External (ERDAS Imagine .aux)"
              ]

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/raster-overview.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Build pyramids (overviews)"
        self.group = "[GDAL] Miscellaneous"
        self.addParameter(ParameterRaster(self.INPUT, "Input layer", False))
        self.addParameter(ParameterString(self.LEVELS, "Overview levels", "2 4 8 16"))
        self.addParameter(ParameterBoolean(self.CLEAN, "Remove all existing overviews", False))
        self.addParameter(ParameterSelection(self.RESAMPLING_METHOD, "Resampling method", self.METHODS, 0))
        self.addParameter(ParameterSelection(self.FORMAT, "Overview format", self.FORMATS, 0))
        self.addOutput(OutputRaster(self.OUTPUT, "Output layer", True))

    def processAlgorithm(self, progress):
        inFile = self.getParameterValue(self.INPUT)
        clearOverviews = self.getParameterValue(self.CLEAN)
        ovrFormat = self.getParameterValue(self.FORMAT)

        arguments = []
        arguments.append(inFile)
        if clearOverviews:
            arguments.append("-clean")
        arguments.append("-r")
        arguments.append(self.METHODS[self.getParameterValue(self.RESAMPLING_METHOD)])

        if ovrFormat == 1:
            # external .ovr
            arguments.append("-ro")
        elif ovrFormat == 2:
            # external .aux
            arguments.extend("--config USE_RRD YES".split(" "))

        arguments.extend(self.getParameterValue(self.LEVELS).split(" "))
        self.setOutputValue(self.OUTPUT, inFile)

        GdalUtils.runGdal(["gdaladdo", GdalUtils.escapeAndJoin(arguments)], progress)
