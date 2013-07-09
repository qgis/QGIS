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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui
from qgis.core import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.outputs.OutputRaster import OutputRaster

from sextante.gdal.GdalUtils import GdalUtils

class translate(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    OUTSIZE = "OUTSIZE"
    OUTSIZE_PERC = "OUTSIZE_PERC"
    NO_DATA = "NO_DATA"
    EXPAND = "EXPAND"
    PROJWIN = "PROJWIN"
    SRS = "SRS"
    SDS = "SDS"
    EXTRA = "EXTRA"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/translate.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Translate (convert format)"
        self.group = "[GDAL] Conversion"
        self.addParameter(ParameterRaster(translate.INPUT, "Input layer", False))
        self.addParameter(ParameterNumber(translate.OUTSIZE, "Set the size of the output file (In pixels or %)", 1, None, 100))
        self.addParameter(ParameterBoolean(translate.OUTSIZE_PERC, "Output size is a percentage of input size", True))
        self.addParameter(ParameterString(translate.NO_DATA, "Nodata value, leave as none to take the nodata value from input", "none"))
        self.addParameter(ParameterSelection(translate.EXPAND, "Expand", ["none","gray","rgb","rgba"]))
        self.addParameter(ParameterCrs(translate.SRS, "Override the projection for the output file", None))
        self.addParameter(ParameterExtent(translate.PROJWIN, "Subset based on georeferenced coordinates"))
        self.addParameter(ParameterBoolean(translate.SDS, "Copy all subdatasets of this file to individual output files", False))
        self.addParameter(ParameterString(translate.EXTRA, "Additional creation parameters", ""))
        self.addOutput(OutputRaster(translate.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        out = self.getOutputValue(translate.OUTPUT)
        outsize = str(self.getParameterValue(translate.OUTSIZE))
        outsizePerc = str(self.getParameterValue(translate.OUTSIZE_PERC))
        noData = str(self.getParameterValue(translate.NO_DATA))
        expand = str(self.getParameterFromName(translate.EXPAND).options[self.getParameterValue(translate.EXPAND)])
        projwin = str(self.getParameterValue(translate.PROJWIN))
        crsId = self.getParameterValue(translate.SRS)
        sds = self.getParameterValue(translate.SDS)
        extra = str(self.getParameterValue(translate.EXTRA))

        arguments = []
        arguments.append("-of")
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        if outsizePerc == "True":
            arguments.append("-outsize")
            arguments.append(outsize + "%")
            arguments.append(outsize + "%")
        else:
            arguments.append("-outsize")
            arguments.append(outsize)
            arguments.append(outsize)
        arguments.append("-a_nodata")
        arguments.append(noData)
        if expand != "none":
            arguments.append("-expand")
            arguments.append(expand)
        regionCoords = projwin.split(",")
        arguments.append("-projwin")
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[3])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[2])
        if crsId is not None:
            arguments.append("-a_srs")
            arguments.append(str(crsId))
            self.crs = QgsCoordinateReferenceSystem(crsId)
        if sds:
            arguments.append("-sds")
        if len(extra) > 0:
            arguments.append(extra)
        arguments.append(self.getParameterValue(translate.INPUT))
        arguments.append(out)

        GdalUtils.runGdal(["gdal_translate", GdalUtils.escapeAndJoin(arguments)], progress)
