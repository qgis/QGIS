# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipByMask.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander bruy at gmail dot com
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
from PyQt4 import QtGui
from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm

from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterString import ParameterString

from processing.outputs.OutputRaster import OutputRaster

from processing.gdal.GdalUtils import GdalUtils

class ClipByMask(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NO_DATA = "NO_DATA"
    MASK = "MASK"
    ALPHA_BAND = "ALPHA_BAND"
    EXTRA = "EXTRA"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/raster-clip.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Clip raster by mask layer"
        self.group = "[GDAL] Extraction"
        self.addParameter(ParameterRaster(self.INPUT, "Input layer", False))
        self.addParameter(ParameterVector(self.MASK, "Mask layer", [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterString(self.NO_DATA, "Nodata value, leave as none to take the nodata value from input", "none"))
        self.addParameter(ParameterBoolean(self.ALPHA_BAND, "Create and output alpha band", False))
        self.addParameter(ParameterString(self.EXTRA, "Additional creation parameters", ""))
        self.addOutput(OutputRaster(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        out = self.getOutputValue(self.OUTPUT)
        mask = self.getParameterValue(self.MASK)
        noData = str(self.getParameterValue(self.NO_DATA))
        addAlphaBand = self.getParameterValue(self.ALPHA_BAND)
        extra = str(self.getParameterValue(self.EXTRA))

        arguments = []
        arguments.append("-q")
        arguments.append("-of")
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.append("-dstnodata")
        arguments.append(noData)

        arguments.append("-cutline")
        arguments.append(mask)
        arguments.append("-crop_to_cutline")

        if addAlphaBand:
            arguments.append("-dstalpha")

        if len(extra) > 0:
            arguments.append(extra)

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        GdalUtils.runGdal(["gdalwarp", GdalUtils.escapeAndJoin(arguments)], progress)
