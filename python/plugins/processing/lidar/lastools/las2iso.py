# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2iso.py
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
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterNumber import ParameterNumber
from processing.outputs.OutputVector import OutputVector
from processing.parameters.ParameterFile import ParameterFile

class las2iso(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CLEAN = "CLEAN"
    SIMPLIFY = "SIMPLIFY"
    INTERVAL = "INTERVAL"

    def defineCharacteristics(self):
        self.name = "las2iso"
        self.group = "Tools"
        self.addParameter(ParameterFile(las2iso.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(las2iso.INTERVAL, "Interval between isolines", 0, None, 10.0))
        self.addParameter(ParameterNumber(las2iso.CLEAN, "Clean isolines shorter than (0 = do not clean)", None, None, 0.0))
        self.addParameter(ParameterNumber(las2iso.SIMPLIFY, "simplify segments shorter than (0 = do not simplify)", None, None, 0.0))
        self.addOutput(OutputVector(las2iso.OUTPUT, "Output isolines"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "las2iso.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(las2iso.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(las2iso.OUTPUT))
        commands.append("-iso_every")
        commands.append(str(self.getParameterValue(las2iso.INTERVAL)))
        simplify = self.getParameterValue(las2iso.SIMPLIFY)
        if simplify != 0:
            commands.append("-simplify")
            commands.append(str(simplify))
        clean = self.getParameterValue(las2iso.CLEAN)
        if clean != 0:
            commands.append("-clean")
            commands.append(str(clean))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
