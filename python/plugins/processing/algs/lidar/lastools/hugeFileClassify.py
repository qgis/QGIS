# -*- coding: utf-8 -*-

"""
***************************************************************************
    hugeFileClassify.py
    ---------------------
    Date                 : May 2014
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
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber

class hugeFileClassify(LAStoolsAlgorithm):

    TILE_SIZE = "TILE_SIZE"
    BUFFER = "BUFFER"
    AIRBORNE = "AIRBORNE"
    TERRAIN = "TERRAIN"
    TERRAINS = ["wilderness", "nature", "town", "city", "metro"]
    GRANULARITY = "GRANULARITY"
    GRANULARITIES = ["coarse", "default", "fine", "extra_fine", "ultra_fine"]

    def defineCharacteristics(self):
        self.name = "hugeFileClassify"
        self.group = "LAStools Pipelines"
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(hugeFileClassify.TILE_SIZE, "tile size (side length of square tile)",  0, None, 1000.0))
        self.addParameter(ParameterNumber(hugeFileClassify.BUFFER, "buffer around each tile (avoids edge artifacts)",  0, None, 25.0))
        self.addParameter(ParameterBoolean(hugeFileClassify.AIRBORNE, "airborne LiDAR", True))
        self.addParameter(ParameterSelection(hugeFileClassify.TERRAIN, "terrain type", hugeFileClassify.TERRAINS, 1))
        self.addParameter(ParameterSelection(hugeFileClassify.GRANULARITY, "preprocessing", hugeFileClassify.GRANULARITIES, 1))
        self.addParametersTemporaryDirectoryGUI()
        self.addParametersPointOutputGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):

#   first we tile the data with option '-reversible'

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        tile_size = self.getParameterValue(hugeFileClassify.TILE_SIZE)
        commands.append("-tile_size")
        commands.append(str(tile_size))
        buffer = self.getParameterValue(hugeFileClassify.BUFFER)
        if buffer != 0.0:
            commands.append("-buffer")
            commands.append(str(buffer))
        commands.append("-reversible")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-o")
        commands.append("hugeFileClassify.laz")

        LAStoolsUtils.runLAStools(commands, progress)

#   then we ground classify the reversible tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasground")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "hugeFileClassify*.laz")
        airborne = self.getParameterValue(hugeFileClassify.AIRBORNE)
        if airborne != True:
            commands.append("-not_airborne")
        method = self.getParameterValue(hugeFileClassify.TERRAIN)
        if method != 1:
            commands.append("-" + hugeFileClassify.TERRAINS[method])
        granularity = self.getParameterValue(hugeFileClassify.GRANULARITY)
        if granularity != 1:
            commands.append("-" + hugeFileClassify.GRANULARITIES[granularity])
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("_g")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we compute the height for each points in the reversible tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "hugeFileClassify*_g.laz")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("h")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we classify buildings and trees in the reversible tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasclassify")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "hugeFileClassify*_gh.laz")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("c")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we reverse the tiling

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "hugeFileClassify*_ghc.laz")
        commands.append("-reverse_tiling")
        self.addParametersPointOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
