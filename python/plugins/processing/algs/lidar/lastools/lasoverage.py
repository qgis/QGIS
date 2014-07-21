# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasoverage.py
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
from processing.core.parameters import ParameterSelection

class lasoverage(LAStoolsAlgorithm):

    CHECK_STEP = "CHECK_STEP"
    OPERATION = "OPERATION"
    OPERATIONS= ["classify as overlap", "flag as withheld", "remove from output"]

    def defineCharacteristics(self):
        self.name = "lasoverage"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersHorizontalFeetGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParameter(ParameterNumber(lasoverage.CHECK_STEP, "size of grid used for scan angle check", 0, None, 1.0))
        self.addParameter(ParameterSelection(lasoverage.OPERATION, "mode of operation", lasoverage.OPERATIONS, 0))
        self.addParametersPointOutputGUI()


    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasoverage.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersHorizontalFeetCommands(commands)
        self.addParametersFilesAreFlightlinesCommands(commands)
        step = self.getParameterValue(lasoverage.CHECK_STEP)
        if step != 1.0:
            commands.append("-step")
            commands.append(str(step))
        operation = self.getParameterValue(lasoverage.OPERATION)
        if operation == 1:
            commands.append("-flag_as_withheld")
        elif operation == 2:
            commands.append("-remove_overage")
        self.addParametersPointOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
