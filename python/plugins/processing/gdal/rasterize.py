# -*- coding: utf-8 -*-

"""
***************************************************************************
    rasterize.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui, QtCore

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools.system import *

from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterNumber import ParameterNumber

from processing.outputs.OutputRaster import OutputRaster

from processing.gdal.GdalUtils import GdalUtils

class rasterize(GeoAlgorithm):

    INPUT = "INPUT"
    FIELD = "FIELD"
    DIMENSIONS = "DIMENSIONS"
    WIDTH = "WIDTH"
    HEIGHT = "HEIGHT"
    OUTPUT = "OUTPUT"



    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/rasterize.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Rasterize"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterVector(self.INPUT, "Input layer"))
        self.addParameter(ParameterTableField(self.FIELD, "Attribute field", self.INPUT))
        self.addParameter(ParameterSelection(self.DIMENSIONS, "Set output raster size", ["Output size in pixels", "Output resolution in map units per pixel"], 0))
        self.addParameter(ParameterNumber(self.WIDTH, "Horizontal", 0.0, 99999999.999999, 3000.0))
        self.addParameter(ParameterNumber(self.HEIGHT, "Vertical", 0.0, 99999999.999999, 3000.0))

        self.addOutput(OutputRaster(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append("-a")
        arguments.append(str(self.getParameterValue(self.FIELD)))

        dimType = self.getParameterValue(self.DIMENSIONS)
        if dimType == 0:
            # size in pixels
            arguments.append("-ts")
        else:
            # resolution in map units per pixel
            arguments.append("-tr")
        arguments.append(str(self.getParameterValue(self.WIDTH)))
        arguments.append(str(self.getParameterValue(self.HEIGHT)))

        arguments.append("-l")
        arguments.append(os.path.basename(os.path.splitext(unicode(self.getParameterValue(self.INPUT)))[0]))
        arguments.append(unicode(self.getParameterValue(self.INPUT)))

        arguments.append(unicode(self.getOutputValue(self.OUTPUT)))

        GdalUtils.runGdal(["gdal_rasterize", GdalUtils.escapeAndJoin(arguments)], progress)
