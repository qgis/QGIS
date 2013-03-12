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

from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.outputs.OutputRaster import OutputRaster
import os
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
        self.addParameter(ParameterString(translate.EXTRA, "Additional creation parameters", " "))
        self.addOutput(OutputRaster(translate.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):

        out = self.getOutputValue(translate.OUTPUT)
        outsize = str(self.getParameterValue(translate.OUTSIZE))
        outsizePerc = str(self.getParameterValue(translate.OUTSIZE_PERC))
        noData = str(self.getParameterValue(translate.NO_DATA))
        expand = str(self.getParameterFromName(translate.EXPAND).options[self.getParameterValue(translate.EXPAND)])
        projwin = str(self.getParameterValue(translate.PROJWIN))
        srs = self.getParameterValue(translate.SRS)
        sds = self.getParameterValue(translate.SDS)
        extra = str(self.getParameterValue(translate.EXTRA))

        commands = ["gdal_translate"]
        commands.append("-of")
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        if outsizePerc == "True":
            outsizeStr = "-outsize "+outsize+"% "+outsize+"%"
        else:
            outsizeStr = "-outsize "+outsize+" "+outsize
        commands.append(outsizeStr)
        commands.append("-a_nodata "+noData)
        if expand != "none":
            commands.append("-expand "+expand)
        regionCoords = projwin.split(",")
        commands.append("-projwin "+regionCoords[0]+" "+regionCoords[3]+" "+regionCoords[1]+" "+regionCoords[2])
        if srs is not None:
            commands.append("-a_srs "+str(srs))
        if sds:
            commands.append("-sds")
        commands.append(extra)
        commands.append(self.getParameterValue(translate.INPUT))
        commands.append(out)


        GdalUtils.runGdal(commands, progress)
