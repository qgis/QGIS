# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasmerge.py
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

from processing.core.parameters import ParameterFile

class lasmerge(LAStoolsAlgorithm):

    FILE2 = "FILE2"
    FILE3 = "FILE3"
    FILE4 = "FILE4"
    FILE5 = "FILE5"
    FILE6 = "FILE6"
    FILE7 = "FILE7"

    def defineCharacteristics(self):
        self.name = "lasmerge"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParametersApplyFileSourceIdGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterFile(lasmerge.FILE2, "2nd file"))
        self.addParameter(ParameterFile(lasmerge.FILE3, "3rd file"))
        self.addParameter(ParameterFile(lasmerge.FILE4, "4th file"))
        self.addParameter(ParameterFile(lasmerge.FILE5, "5th file"))
        self.addParameter(ParameterFile(lasmerge.FILE6, "6th file"))
        self.addParameter(ParameterFile(lasmerge.FILE7, "7th file"))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasmerge")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        file2 = self.getParameterValue(lasmerge.FILE2)
        if file2 != None:
            commands.append("-i")
            commands.append(file2)
        file3 = self.getParameterValue(lasmerge.FILE3)
        if file3 != None:
            commands.append("-i")
            commands.append(file3)
        file4 = self.getParameterValue(lasmerge.FILE4)
        if file4 != None:
            commands.append("-i")
            commands.append(file4)
        file5 = self.getParameterValue(lasmerge.FILE5)
        if file5 != None:
            commands.append("-i")
            commands.append(file5)
        file6 = self.getParameterValue(lasmerge.FILE6)
        if file6 != None:
            commands.append("-i")
            commands.append(file6)
        file7 = self.getParameterValue(lasmerge.FILE7)
        if file7 != None:
            commands.append("-i")
            commands.append(file7)
        self.addParametersFilesAreFlightlinesCommands(commands)
        self.addParametersApplyFileSourceIdsCommands(commands)
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
