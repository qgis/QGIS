# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasindexPro.py
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


class lasindexPro(LAStoolsAlgorithm):

    MOBILE_OR_TERRESTRIAL = "MOBILE_OR_TERRESTRIAL"
    APPEND_LAX = "APPEND_LAX"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasindexPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterBoolean(lasindexPro.APPEND_LAX,
                                           self.tr("append *.lax file to *.laz file"), False))
        self.addParameter(ParameterBoolean(lasindexPro.MOBILE_OR_TERRESTRIAL,
                                           self.tr("is mobile or terrestrial LiDAR (not airborne)"), False))
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasindex")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        if self.getParameterValue(lasindexPro.APPEND_LAX):
            commands.append("-append")
        if self.getParameterValue(lasindexPro.MOBILE_OR_TERRESTRIAL):
            commands.append("-tile_size")
            commands.append("10")
            commands.append("-maximum")
            commands.append("-100")
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
