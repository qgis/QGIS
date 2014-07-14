# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasthin.py
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
from processing.core.parameters import ParameterSelection

class lasthin(LAStoolsAlgorithm):

    THIN_STEP = "THIN_STEP"
    OPERATION = "OPERATION"
    OPERATIONS= ["lowest", "random", "highest"]
    WITHHELD = "WITHHELD"

    def defineCharacteristics(self):
        self.name = "lasthin"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(lasthin.THIN_STEP, "size of grid used for thinning", 0, None, 1.0))
        self.addParameter(ParameterSelection(lasthin.OPERATION, "keep particular point per cell", lasthin.OPERATIONS, 0))
        self.addParameter(ParameterBoolean(lasthin.WITHHELD, "mark points as withheld", False))
        self.addParametersPointOutputGUI()


    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasthin.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        step = self.getParameterValue(lasthin.THIN_STEP)
        if step != 0.0:
            commands.append("-step")
            commands.append(str(step))
        operation = self.getParameterValue(lasthin.OPERATION)
        if operation != 0:
            commands.append("-" + self.OPERATIONS[operation])
        if self.getParameterValue(lasthin.WITHHELD):
            commands.append("-withheld")
        self.addParametersPointOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
