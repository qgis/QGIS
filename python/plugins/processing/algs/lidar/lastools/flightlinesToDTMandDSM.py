# -*- coding: utf-8 -*-

"""
***************************************************************************
    flightlinesToDTMandDSM.py
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

__author__ = 'Martin Isenburg'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString

class flightlinesToDTMandDSM(LAStoolsAlgorithm):

    TILE_SIZE = "TILE_SIZE"
    BUFFER = "BUFFER"
    TERRAIN = "TERRAIN"
    TERRAINS = ["wilderness", "nature", "town", "city", "metro"]
    BASE_NAME = "BASE_NAME"

    def defineCharacteristics(self):
        self.name = "flightlinesToDTMandDSM"
        self.group = "LAStools Pipelines"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterNumber(flightlinesToDTMandDSM.TILE_SIZE, "tile size (side length of square tile)",  0, None, 1000.0))
        self.addParameter(ParameterNumber(flightlinesToDTMandDSM.BUFFER, "buffer around each tile (avoids edge artifacts)",  0, None, 25.0))
        self.addParameter(ParameterSelection(flightlinesToDTMandDSM.TERRAIN, "terrain type", flightlinesToDTMandDSM.TERRAINS, 1))
        self.addParametersStepGUI()
        self.addParametersTemporaryDirectoryGUI()
        self.addParametersOutputDirectoryGUI()
        self.addParameter(ParameterString(flightlinesToDTMandDSM.BASE_NAME, "tile base name (using 'sydney' creates sydney_274000_4714000...)","tile"))
        self.addParametersRasterOutputFormatGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):

#   first we tile the data

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        commands.append("-files_are_flightlines")
        tile_size = self.getParameterValue(flightlinesToDTMandDSM.TILE_SIZE)
        commands.append("-tile_size")
        commands.append(str(tile_size))
        buffer = self.getParameterValue(flightlinesToDTMandDSM.BUFFER)
        if buffer != 0.0:
            commands.append("-buffer")
            commands.append(str(buffer))
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        base_name = self.getParameterValue(flightlinesToDTMandDSM.BASE_NAME)
        if base_name == "":
            base_name = "tile"
        commands.append("-o")
        commands.append(base_name)
        commands.append("-olaz")

        LAStoolsUtils.runLAStools(commands, progress)

#   then we ground classify the tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasground.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*.laz")
        method = self.getParameterValue(flightlinesToDTMandDSM.TERRAIN)
        if method != 1:
            commands.append("-" + flightlinesToDTMandDSM.TERRAINS[method])
        if method > 2:
            commands.append("-ultra_fine")
        elif method > 1:
            commands.append("-extra_fine")
        elif method > 0:
            commands.append("-fine")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("_g")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we rasterize the classified tiles into DTMs

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*_g.laz")
        commands.append("-keep_class")
        commands.append("2")
        self.addParametersStepCommands(commands)
        commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("2")
        commands.append("-odix")
        commands.append("_dtm")
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we rasterize the classified tiles into DSMs

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*_g.laz")
        commands.append("-first_only")
        self.addParametersStepCommands(commands)
        commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("2")
        commands.append("-odix")
        commands.append("_dsm")
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
