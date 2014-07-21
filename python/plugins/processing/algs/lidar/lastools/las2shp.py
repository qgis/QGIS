# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2shp.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Martin Isenburg
    Email                : martin near rapidlasso point com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Martin Isenburg'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputFile

class las2shp(LAStoolsAlgorithm):

    POINT_Z = "POINT_Z"
    RECORD_SIZE = "RECORD_SIZE"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "las2shp"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(las2shp.POINT_Z, "use PointZ instead of MultiPointZ", False))
        self.addParameter(ParameterNumber(las2shp.RECORD_SIZE, "number of points per record", 0, None, 1024))
        self.addOutput(OutputFile(las2shp.OUTPUT, "Output SHP file"))

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2shp.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(las2shp.POINT_Z):
            commands.append("-single_points")
        record_size = self.getParameterValue(las2shp.RECORD_SIZE)
        if record_size != 1024:
            commands.append("-record_size")
            commands.append(str(record_size))
        commands.append("-o")
        commands.append(self.getOutputValue(las2shp.OUTPUT))

        LAStoolsUtils.runLAStools(commands, progress)
