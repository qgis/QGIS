# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasoveragePro.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Martin Isenburg
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection

class lasoveragePro(LAStoolsAlgorithm):

    CHECK_STEP = "CHECK_STEP"
    OPERATION = "OPERATION"
    OPERATIONS= ["classify as overlap", "flag as withheld", "remove from output"]

    def defineCharacteristics(self):
        self.name = "lasoveragePro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParametersHorizontalFeetGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParameter(ParameterNumber(lasoveragePro.CHECK_STEP,
            self.tr("size of grid used for scan angle check"), 0, None, 1.0))
        self.addParameter(ParameterSelection(lasoveragePro.OPERATION,
            self.tr("mode of operation"), lasoveragePro.OPERATIONS, 0))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasoverage")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersHorizontalFeetCommands(commands)
        self.addParametersFilesAreFlightlinesCommands(commands)
        step = self.getParameterValue(lasoveragePro.CHECK_STEP)
        if step != 1.0:
            commands.append("-step")
            commands.append(str(step))
        operation = self.getParameterValue(lasoveragePro.OPERATION)
        if operation == 1:
            commands.append("-flag_as_withheld")
        elif operation == 2:
            commands.append("-remove_overage")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
