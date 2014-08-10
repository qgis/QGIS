# -*- coding: utf-8 -*-

"""
***************************************************************************
    flightlinesToCHM.py
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

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString

class flightlinesToCHM(LAStoolsAlgorithm):

    TILE_SIZE = "TILE_SIZE"
    BUFFER = "BUFFER"
    TERRAIN = "TERRAIN"
    TERRAINS = ["wilderness", "nature", "town", "city", "metro"]
    BEAM_WIDTH = "BEAM_WIDTH"
    BASE_NAME = "BASE_NAME"

    def defineCharacteristics(self):
        self.name = "flightlinesToCHM"
        self.group = "LAStools Pipelines"
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterNumber(flightlinesToCHM.TILE_SIZE, "tile size (side length of square tile)",  0, None, 1000.0))
        self.addParameter(ParameterNumber(flightlinesToCHM.BUFFER, "buffer around each tile (avoids edge artifacts)",  0, None, 25.0))
        self.addParameter(ParameterSelection(flightlinesToCHM.TERRAIN, "terrain type", flightlinesToCHM.TERRAINS, 1))
        self.addParameter(ParameterNumber(flightlinesToCHM.BEAM_WIDTH, "laser beam width (diameter of laser footprint)",  0, None, 0.2))
        self.addParametersStepGUI()
        self.addParametersTemporaryDirectoryGUI()
        self.addParametersOutputDirectoryGUI()
        self.addParameter(ParameterString(flightlinesToCHM.BASE_NAME, "tile base name (using 'sydney' creates sydney_274000_4714000...)","tile"))
        self.addParametersRasterOutputFormatGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):

#   first we tile the data

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        commands.append("-files_are_flightlines")
        tile_size = self.getParameterValue(flightlinesToCHM.TILE_SIZE)
        commands.append("-tile_size")
        commands.append(str(tile_size))
        buffer = self.getParameterValue(flightlinesToCHM.BUFFER)
        if buffer != 0.0:
            commands.append("-buffer")
            commands.append(str(buffer))
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        base_name = self.getParameterValue(flightlinesToCHM.BASE_NAME)
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
        method = self.getParameterValue(flightlinesToCHM.TERRAIN)
        if method != 1:
            commands.append("-" + flightlinesToCHM.TERRAINS[method])
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

#   then we height-normalize the tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*_g.laz")
        commands.append("-replace_z")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("h")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we thin and splat the tiles

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasthin.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*_gh.laz")
        beam_width = self.getParameterValue(flightlinesToCHM.BEAM_WIDTH)
        if beam_width != 0.0:
            commands.append("-subcircle")
            commands.append(str(beam_width/2))
        step = self.getParametersStepValue()
        commands.append("-step")
        commands.append(str(step/4))
        commands.append("-highest")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("t")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

#   then we rasterize the classified tiles into CHMs

        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, base_name+"*_ght.laz")
        self.addParametersStepCommands(commands)
        commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm")
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
