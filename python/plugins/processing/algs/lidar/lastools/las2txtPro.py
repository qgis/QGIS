# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2txtPro.py
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

from processing.core.parameters import ParameterString

class las2txtPro(LAStoolsAlgorithm):

    PARSE = "PARSE"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('las2txtPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterString(las2txtPro.PARSE,
            self.tr("parse string"), "xyz"))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2txt")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        parse = self.getParameterValue(las2txtPro.PARSE)
        if parse != "xyz":
            commands.append("-parse")
            commands.append(parse)
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        commands.append("-otxt")
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
