# -*- coding: utf-8 -*-

"""
***************************************************************************
    flightlinesToSingleCHMpitFree.py
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

class flightlinesToSingleCHMpitFree(LAStoolsAlgorithm):

    TILE_SIZE = "TILE_SIZE"
    BUFFER = "BUFFER"
    TERRAIN = "TERRAIN"
    TERRAINS = ["wilderness", "nature", "town", "city", "metro"]
    BEAM_WIDTH = "BEAM_WIDTH"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('flightlinesToSingleCHMpitFree')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Pipelines')
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterNumber(flightlinesToSingleCHMpitFree.TILE_SIZE,
            self.tr("tile size (side length of square tile)"),
            0, None, 1000.0))
        self.addParameter(ParameterNumber(flightlinesToSingleCHMpitFree.BUFFER,
            self.tr("buffer around each tile (avoids edge artifacts)"),
            0, None, 25.0))
        self.addParameter(ParameterSelection(flightlinesToSingleCHMpitFree.TERRAIN,
            self.tr("terrain type"), flightlinesToSingleCHMpitFree.TERRAINS, 1))
        self.addParameter(ParameterNumber(flightlinesToSingleCHMpitFree.BEAM_WIDTH,
            self.tr("laser beam width (diameter of laser footprint)"), 0, None, 0.2))
        self.addParametersStepGUI()
        self.addParametersTemporaryDirectoryGUI()
        self.addParametersRasterOutputGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        # needed for thinning and killing
        step = self.getParametersStepValue()

        # first we tile the data
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lastile")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        commands.append("-files_are_flightlines")
        tile_size = self.getParameterValue(flightlinesToSingleCHMpitFree.TILE_SIZE)
        commands.append("-tile_size")
        commands.append(str(tile_size))
        buffer = self.getParameterValue(flightlinesToSingleCHMpitFree.BUFFER)
        if buffer != 0.0:
            commands.append("-buffer")
            commands.append(str(buffer))
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-o")
        commands.append("tile.laz")

        LAStoolsUtils.runLAStools(commands, progress)

        # then we ground classify the tiles
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasground")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*.laz")
        method = self.getParameterValue(flightlinesToSingleCHMpitFree.TERRAIN)
        if method != 1:
            commands.append("-" + flightlinesToSingleCHMpitFree.TERRAINS[method])
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

        # then we height-normalize the tiles
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_g.laz")
        commands.append("-replace_z")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("h")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we thin and splat the tiles
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasthin")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_gh.laz")
        beam_width = self.getParameterValue(flightlinesToSingleCHMpitFree.BEAM_WIDTH)
        if beam_width != 0.0:
            commands.append("-subcircle")
            commands.append(str(beam_width/2))
        commands.append("-step")
        commands.append(str(step/4))
        commands.append("-highest")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-odix")
        commands.append("t")
        commands.append("-olaz")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 00
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        self.addParametersStepCommands(commands)
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm00")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 02
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        commands.append("-drop_z_below")
        commands.append("2")
        self.addParametersStepCommands(commands)
        commands.append("-kill")
        commands.append(str(step*3))
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm02")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 05
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        commands.append("-drop_z_below")
        commands.append("5")
        self.addParametersStepCommands(commands)
        commands.append("-kill")
        commands.append(str(step*3))
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm05")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 10
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        commands.append("-drop_z_below")
        commands.append("10")
        self.addParametersStepCommands(commands)
        commands.append("-kill")
        commands.append(str(step*3))
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm10")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 15
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        commands.append("-drop_z_below")
        commands.append("15")
        self.addParametersStepCommands(commands)
        commands.append("-kill")
        commands.append(str(step*3))
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm15")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we rasterize the classified tiles into the partial CHMs at level 20
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile*_ght.laz")
        commands.append("-drop_z_below")
        commands.append("20")
        self.addParametersStepCommands(commands)
        commands.append("-kill")
        commands.append(str(step*3))
        commands.append("-use_tile_bb")
        self.addParametersTemporaryDirectoryAsOutputDirectoryCommands(commands)
        commands.append("-ocut")
        commands.append("4")
        commands.append("-odix")
        commands.append("_chm20")
        commands.append("-obil")
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)

        # then we combine the partial CHMs into a single output CHM
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasgrid")]
        self.addParametersVerboseCommands(commands)
        self.addParametersTemporaryDirectoryAsInputFilesCommands(commands, "tile_chm*.bil")
        commands.append("-highest")
        self.addParametersStepCommands(commands)
        self.addParametersRasterOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
