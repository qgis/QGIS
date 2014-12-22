# -*- coding: utf-8 -*-

"""
***************************************************************************
    lastile.py
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

from processing.core.parameters import ParameterNumber

class lastile(LAStoolsAlgorithm):

    TILE_SIZE = "TILE_SIZE"
    BUFFER = "BUFFER"
    REVERSIBLE = "REVERSIBLE"

    def defineCharacteristics(self):
        self.name = "lastile"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParametersApplyFileSourceIdGUI()
        self.addParameter(ParameterNumber(lastile.TILE_SIZE, "tile size (side length of square tile)",  None, None, 1000.0))
        self.addParameter(ParameterNumber(lastile.BUFFER, "buffer around each tile",  None, None, 0.0))
        self.addParameter(ParameterNumber(lastile.REVERSIBLE, "make tiling reversible (advanced, usually not needed)", False))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersFilesAreFlightlinesCommands(commands)
        self.addParametersApplyFileSourceIdCommands(commands)
        tile_size = self.getParameterValue(lastile.TILE_SIZE)
        commands.append("-tile_size")
        commands.append(str(tile_size))
        buffer = self.getParameterValue(lastile.BUFFER)
        if buffer != 0.0:
            commands.append("-buffer")
            commands.append(str(buffer))
        if self.getParameterValue(lastile.REVERSIBLE):
            commands.append("-reversible")
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
