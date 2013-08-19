# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasgrid.py
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
from processing.parameters.ParameterString import ParameterString
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.outputs.OutputRaster import OutputRaster
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterFile import ParameterFile

class lasgrid(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    INTENSITY = "INTENSITY"
    METHOD = "METHOD"
    METHODS = ["-average", "-lowest", "-highest", "-stddev"]

    def defineCharacteristics(self):
        self.name = "lasgrid"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasgrid.INPUT, "Input las layer"))
        self.addParameter(ParameterBoolean(lasgrid.INTENSITY, "Use intensity instead of elevation", False))
        self.addParameter(ParameterSelection(lasgrid.METHOD, "Method", lasgrid.METHODS))
        self.addOutput(OutputRaster(lasgrid.OUTPUT, "Output grid layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasgrid.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasgrid.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasgrid.OUTPUT))
        if self.getParameterValue(lasgrid.INTENSITY):
            commands.append("-intensity")
        commands.append(lasgrid.METHODS[self.getParameterValue(lasgrid.METHOD)])
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
