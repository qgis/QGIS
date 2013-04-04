# -*- coding: utf-8 -*-

"""
***************************************************************************
    information.py
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

from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterBoolean import ParameterBoolean
import os
from sextante.gdal.GdalUtils import GdalUtils
from sextante.outputs.OutputHTML import OutputHTML

class information(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NOGCP = "NOGCP"
    NOMETADATA = "NOMETADATA"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/raster-info.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "Information"
        self.group = "[GDAL] Miscellaneous"
        self.addParameter(ParameterRaster(information.INPUT, "Input layer", False))
        self.addParameter(ParameterBoolean(information.NOGCP, "Suppress GCP info", False))
        self.addParameter(ParameterBoolean(information.NOMETADATA, "Suppress metadata info", False))
        self.addOutput(OutputHTML(information.OUTPUT, "Layer information"))

    def processAlgorithm(self, progress):
        commands = ["gdalinfo"]
        if self.getParameterValue(information.NOGCP):
            commands.append("-nogcp")
        if self.getParameterValue(information.NOMETADATA):
            commands.append("-nomd")
        commands.append(self.getParameterValue(information.INPUT))
        GdalUtils.runGdal(commands, progress)
        output = self.getOutputValue(information.OUTPUT)
        f = open(output, "w")
        for s in GdalUtils.getConsoleOutput()[1:]:
            f.write("<p>" + str(s) + "</p>")
        f.close()
