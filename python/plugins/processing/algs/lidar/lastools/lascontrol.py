# -*- coding: utf-8 -*-

"""
***************************************************************************
    lascontrol.py
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

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection

class lascontrol(LAStoolsAlgorithm):

    POLYGON = "POLYGON"
    INTERIOR = "INTERIOR"
    OPERATION = "OPERATION"
    OPERATIONS = ["clip", "classify"]
    CLASSIFY_AS = "CLASSIFY_AS"

    def defineCharacteristics(self):
        self.name = "lascontrol"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterVector(lascontrol.POLYGON,
            self.tr("Input polygon(s)"), ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterBoolean(lascontrol.INTERIOR,
            self.tr("interior"), False))
        self.addParameter(ParameterSelection(lascontrol.OPERATION,
            self.tr("what to do with isolated points"), lascontrol.OPERATIONS, 0))
        self.addParameter(ParameterNumber(lascontrol.CLASSIFY_AS,
            self.tr("classify as"), 0, None, 12))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lascontrol")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        poly = self.getParameterValue(lascontrol.POLYGON)
        if poly is not None:
            commands.append("-poly")
            commands.append(poly)
        if self.getParameterValue(lascontrol.INTERIOR):
            commands.append("-interior")
        operation = self.getParameterValue(lascontrol.OPERATION)
        if operation != 0:
            commands.append("-classify")
            classify_as = self.getParameterValue(lascontrol.CLASSIFY_AS)
            commands.append(str(classify_as))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
