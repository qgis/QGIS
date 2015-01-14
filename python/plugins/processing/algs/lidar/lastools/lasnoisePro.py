# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasnoisePro.py
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

class lasnoisePro(LAStoolsAlgorithm):

    ISOLATED = "ISOLATED"
    STEP_XY = "STEP_XY"
    STEP_Z = "STEP_Z"
    OPERATION = "OPERATION"
    OPERATIONS = ["classify", "remove"]
    CLASSIFY_AS = "CLASSIFY_AS"

    def defineCharacteristics(self):
        self.name = "lasnoisePro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterNumber(lasnoisePro.ISOLATED,
            self.tr("isolated if surrounding cells have only"), 0, None, 5))
        self.addParameter(ParameterNumber(lasnoisePro.STEP_XY,
            self.tr("resolution of isolation grid in xy"), 0, None, 4.0))
        self.addParameter(ParameterNumber(lasnoisePro.STEP_Z,
            self.tr("resolution of isolation grid in z"), 0, None, 4.0))
        self.addParameter(ParameterSelection(lasnoisePro.OPERATION,
            self.tr("what to do with isolated points"), lasnoisePro.OPERATIONS, 0))
        self.addParameter(ParameterNumber(lasnoisePro.CLASSIFY_AS,
            self.tr("classify as"), 0, None, 7))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()


    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasnoise")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        isolated = self.getParameterValue(lasnoisePro.ISOLATED)
        commands.append("-isolated")
        commands.append(str(isolated))
        step_xy = self.getParameterValue(lasnoisePro.STEP_XY)
        commands.append("-step_xy")
        commands.append(str(step_xy))
        step_z = self.getParameterValue(lasnoisePro.STEP_Z)
        commands.append("-step_z")
        commands.append(str(step_z))
        operation = self.getParameterValue(lasnoisePro.OPERATION)
        if operation != 0:
            commands.append("-remove_noise")
        else:
            commands.append("-classify_as")
            classify_as = self.getParameterValue(lasnoisePro.CLASSIFY_AS)
            commands.append(str(classify_as))
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
