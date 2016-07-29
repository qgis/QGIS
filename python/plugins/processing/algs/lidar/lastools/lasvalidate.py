# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasvalidate.py
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
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile


class lasvalidate(LAStoolsAlgorithm):

    ONE_REPORT_PER_FILE = "ONE_REPORT_PER_FILE"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasvalidate')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(lasvalidate.ONE_REPORT_PER_FILE,
                                           self.tr("save report to '*_LVS.xml'"), False))
        self.addOutput(OutputFile(lasvalidate.OUTPUT, self.tr("Output XML file")))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasvalidate")]
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(lasvalidate.ONE_REPORT_PER_FILE):
            commands.append("-oxml")
        else:
            commands.append("-o")
            commands.append(self.getOutputValue(lasvalidate.OUTPUT))
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
