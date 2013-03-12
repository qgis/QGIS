# -*- coding: utf-8 -*-

"""
***************************************************************************
    warp.py
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
from qgis.core import *
from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.gdal.GdalUtils import GdalUtils
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterString import ParameterString

class warp(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    SOURCE_SRS = "SOURCE_SRS"
    DEST_SRS = "DEST_SRS "
    METHOD = "METHOD"
    METHOD_OPTIONS = ["near", "bilinear", "cubic", "cubicspline", "lanczos"]
    TR = "TR"
    EXTRA = "EXTRA"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/warp.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Warp (reproject)"
        self.group = "[GDAL] Projections"
        self.addParameter(ParameterRaster(warp.INPUT, "Input layer", False))
        self.addParameter(ParameterCrs(warp.SOURCE_SRS, "Source SRS (EPSG Code)", "EPSG:4326"))
        self.addParameter(ParameterCrs(warp.DEST_SRS, "Destination SRS (EPSG Code)", "EPSG:4326"))
        self.addParameter(ParameterNumber(warp.TR, "Output file resolution in target georeferenced units (leave 0 for no change)", 0.0, None, 0.0))
        self.addParameter(ParameterSelection(warp.METHOD, "Resampling method", warp.METHOD_OPTIONS))
        self.addParameter(ParameterString(warp.EXTRA, "Additional creation parameters", " "))
        self.addOutput(OutputRaster(warp.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        srs = self.getParameterValue(warp.DEST_SRS)
        self.crs = QgsCoordinateReferenceSystem(srs)
        commands = ["gdalwarp"]
        commands.append("-s_srs")
        commands.append(str(self.getParameterValue(warp.SOURCE_SRS)))
        commands.append("-t_srs")
        commands.append(str(srs))
        commands.append("-r")
        commands.append(warp.METHOD_OPTIONS[self.getParameterValue(warp.METHOD)])
        commands.append("-of")
        out = self.getOutputValue(warp.OUTPUT)
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        if self.getParameterValue(warp.TR) != 0:
            trStr = "-tr "+str(self.getParameterValue(warp.TR))+" "+str(self.getParameterValue(warp.TR))
            commands.append(trStr)
        commands.append(str(self.getParameterValue(warp.EXTRA)))
        commands.append(self.getParameterValue(warp.INPUT))
        commands.append(out)

        GdalUtils.runGdal(commands, progress)
