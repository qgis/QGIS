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

from processing.core.parameters import ParameterString
from processing.core.outputs import OutputFile


class las2txt(LAStoolsAlgorithm):

    PARSE = "PARSE"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('las2txt')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterString(las2txt.PARSE,
                                          self.tr("parse string"), "xyz"))
        self.addOutput(OutputFile(las2txt.OUTPUT, self.tr("Output ASCII file")))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2txt")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        parse = self.getParameterValue(las2txt.PARSE)
        if parse != "xyz":
            commands.append("-parse")
            commands.append(parse)
        commands.append("-o")
        commands.append(self.getOutputValue(las2txt.OUTPUT))
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
