# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasheightPro.py
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
from processing.core.parameters import ParameterNumber

class lasheightPro(LAStoolsAlgorithm):

    REPLACE_Z = "REPLACE_Z"
    DROP_ABOVE = "DROP_ABOVE"
    DROP_ABOVE_HEIGHT = "DROP_ABOVE_HEIGHT"
    DROP_BELOW = "DROP_BELOW"
    DROP_BELOW_HEIGHT = "DROP_BELOW_HEIGHT"

    def defineCharacteristics(self):
        self.name = "lasheightPro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterBoolean(lasheightPro.REPLACE_Z,
            self.tr("replace z"), False))
        self.addParameter(ParameterBoolean(lasheightPro.DROP_ABOVE,
            self.tr("drop above"), False))
        self.addParameter(ParameterNumber(lasheightPro.DROP_ABOVE_HEIGHT,
            self.tr("drop above height"), 0, None, 100.0))
        self.addParameter(ParameterBoolean(lasheightPro.DROP_BELOW,
            self.tr("drop below"), False))
        self.addParameter(ParameterNumber(lasheightPro.DROP_BELOW_HEIGHT,
            self.tr("drop below height"), 0, None, -2.0))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        if self.getParameterValue(lasheightPro.REPLACE_Z):
            commands.append("-replace_z")
        if self.getParameterValue(lasheightPro.DROP_ABOVE):
            commands.append("-drop_above")
            commands.append(str(self.getParameterValue(lasheightPro.DROP_ABOVE_HEIGHT)))
        if self.getParameterValue(lasheightPro.DROP_BELOW):
            commands.append("-drop_below")
            commands.append(str(self.getParameterValue(lasheightPro.DROP_BELOW_HEIGHT)))
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
