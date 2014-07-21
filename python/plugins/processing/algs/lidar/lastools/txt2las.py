# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2txt.py
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

from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterFile

class txt2las(LAStoolsAlgorithm):

    INPUT = "INPUT"
    PARSE_STRING = "PARSE_STRING"
    SKIP = "SKIP"
    SCALE_FACTOR_XY = "SCALE_FACTOR_XY"
    SCALE_FACTOR_Z = "SCALE_FACTOR_Z"

    def defineCharacteristics(self):
        self.name = "txt2las"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParameter(ParameterFile(txt2las.INPUT, "Input ASCII file"))
        self.addParameter(ParameterString(txt2las.PARSE_STRING, "parse lines as", "xyz"))
        self.addParameter(ParameterNumber(txt2las.SKIP, "skip the first n lines", False, False, 0))
        self.addParameter(ParameterNumber(txt2las.SCALE_FACTOR_XY, "resolution of x and y coordinate", False, False, 0.01))
        self.addParameter(ParameterNumber(txt2las.SCALE_FACTOR_Z, "resolution of z coordinate", False, False, 0.01))
        self.addParametersPointOutputGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "txt2las.exe")]
        self.addParametersVerboseCommands(commands)
        commands.append("-i")
        commands.append(self.getParameterValue(txt2las.INPUT))
        parse_string = self.getParameterValue(txt2las.PARSE_STRING)
        if parse_string != "xyz":
            commands.append("-parse_string")
            commands.append(parse_string)
        skip = self.getParameterValue(txt2las.SKIP)
        if parse_string != 0:
            commands.append("-skip")
            commands.append(str(skip))
        scale_factor_xy = self.getParameterValue(txt2las.SCALE_FACTOR_XY)
        scale_factor_z = self.getParameterValue(txt2las.SCALE_FACTOR_Z)
        if scale_factor_xy != 0.01 or scale_factor_z != 0.01:
            commands.append("-set_scale_factor")
            commands.append(str(scale_factor_xy) + " " + str(scale_factor_xy) + " " + str(scale_factor_z))
        self.addParametersPointOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
