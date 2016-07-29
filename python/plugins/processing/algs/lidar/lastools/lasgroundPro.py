# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasgroundPro.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : April 2014
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection


class lasgroundPro(LAStoolsAlgorithm):

    NO_BULGE = "NO_BULGE"
    TERRAIN = "TERRAIN"
    TERRAINS = ["wilderness", "nature", "town", "city", "metro"]
    GRANULARITY = "GRANULARITY"
    GRANULARITIES = ["coarse", "default", "fine", "extra_fine", "ultra_fine"]

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasgroundPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParametersHorizontalAndVerticalFeetGUI()
        self.addParameter(ParameterBoolean(lasgroundPro.NO_BULGE,
                                           self.tr("no triangle bulging during TIN refinement"), False))
        self.addParameter(ParameterSelection(lasgroundPro.TERRAIN,
                                             self.tr("terrain type"), lasgroundPro.TERRAINS, 1))
        self.addParameter(ParameterSelection(lasgroundPro.GRANULARITY,
                                             self.tr("preprocessing"), lasgroundPro.GRANULARITIES, 1))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasground")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersHorizontalAndVerticalFeetCommands(commands)
        if (self.getParameterValue(lasgroundPro.NO_BULGE)):
            commands.append("-no_bulge")
        method = self.getParameterValue(lasgroundPro.TERRAIN)
        if (method != 1):
            commands.append("-" + lasgroundPro.TERRAINS[method])
        granularity = self.getParameterValue(lasgroundPro.GRANULARITY)
        if (granularity != 1):
            commands.append("-" + lasgroundPro.GRANULARITIES[granularity])
        self.addParametersCoresCommands(commands)
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
