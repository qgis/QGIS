# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasviewPro.py
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

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber

class lasviewPro(LAStoolsAlgorithm):

    POINTS = "POINTS"

    SIZE = "SIZE"
    SIZES = ["1024 768", "800 600", "1200 900", "1200 400", "1550 900", "1550 1150"]

    COLORING = "COLORING"
    COLORINGS = ["default", "classification", "elevation1", "elevation2", "intensity", "return", "flightline", "rgb"]

    def defineCharacteristics(self):
        self.name = "lasviewPro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParameter(ParameterNumber(lasviewPro.POINTS,
            self.tr("max number of points sampled"), 100000, 20000000, 5000000))
        self.addParameter(ParameterSelection(lasviewPro.COLORING,
            self.tr("color by"), lasviewPro.COLORINGS, 0))
        self.addParameter(ParameterSelection(lasviewPro.SIZE,
            self.tr("window size (x y) in pixels"), lasviewPro.SIZES, 0))
        self.addParametersAdditionalGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasview")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersFilesAreFlightlinesCommands(commands)
        points = self.getParameterValue(lasviewPro.POINTS)
        commands.append("-points " + str(points))
        self.addParametersAdditionalCommands(commands)
        coloring = self.getParameterValue(lasviewPro.COLORING)
        if coloring != 0:
            commands.append("-color_by_" + lasviewPro.COLORINGS[coloring])
        size = self.getParameterValue(lasviewPro.SIZE)
        if size != 0:
            commands.append("-win " + lasviewPro.SIZES[size])

        LAStoolsUtils.runLAStools(commands, progress)
