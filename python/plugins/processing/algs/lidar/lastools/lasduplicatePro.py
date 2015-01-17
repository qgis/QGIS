# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasduplicate.py
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

class lasduplicatePro(LAStoolsAlgorithm):

    LOWEST_Z = "LOWEST_Z"
    UNIQUE_XYZ = "UNIQUE_XYZ"
    SINGLE_RETURNS = "SINGLE_RETURNS"
    RECORD_REMOVED = "RECORD_REMOVED"

    def defineCharacteristics(self):
        self.name = "lasduplicatePro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterBoolean(lasduplicatePro.LOWEST_Z,
            self.tr("keep duplicate with lowest z coordinate"), False))
        self.addParameter(ParameterBoolean(lasduplicatePro.UNIQUE_XYZ,
            self.tr("only remove duplicates in x y and z"), False))
        self.addParameter(ParameterBoolean(lasduplicatePro.SINGLE_RETURNS,
            self.tr("mark surviving duplicate as single return"), False))
        self.addParameter(ParameterBoolean(lasduplicatePro.RECORD_REMOVED,
            self.tr("record removed duplicates"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()


    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasduplicate")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        if self.getParameterValue(lasduplicatePro.LOWEST_Z):
            commands.append("-lowest_z")
        if self.getParameterValue(lasduplicatePro.UNIQUE_XYZ):
            commands.append("-unique_xyz")
        if self.getParameterValue(lasduplicatePro.SINGLE_RETURNS):
            commands.append("-single_returns")
        if self.getParameterValue(lasduplicatePro.RECORD_REMOVED):
            commands.append("-record_removed")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
