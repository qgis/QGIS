# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasvalidatePro.py
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

from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile

class lasvalidatePro(LAStoolsAlgorithm):

    ONE_REPORT_PER_FILE = "ONE_REPORT_PER_FILE"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasvalidatePro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterBoolean(lasvalidatePro.ONE_REPORT_PER_FILE,
            self.tr("generate one '*_LVS.xml' report per file"), False))
        self.addOutput(OutputFile(lasvalidatePro.OUTPUT, self.tr("Output XML file")))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasvalidate")]
        self.addParametersPointInputFolderCommands(commands)
        if self.getParameterValue(lasvalidatePro.ONE_REPORT_PER_FILE):
            commands.append("-oxml")
        else:
            commands.append("-o")
            commands.append(self.getOutputValue(lasvalidatePro.OUTPUT))
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
