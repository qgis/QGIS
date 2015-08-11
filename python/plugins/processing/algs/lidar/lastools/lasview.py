# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasview.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
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

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber

class lasview(LAStoolsAlgorithm):

    POINTS = "POINTS"

    SIZE = "SIZE"
    SIZES = ["1024 768", "800 600", "1200 900", "1200 400", "1550 900", "1550 1150"]

    COLORING = "COLORING"
    COLORINGS = ["default", "classification", "elevation1", "elevation2", "intensity", "return", "flightline", "rgb"]

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasview')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(lasview.POINTS,
            self.tr("max number of points sampled"), 100000, 20000000, 5000000))
        self.addParameter(ParameterSelection(lasview.COLORING,
            self.tr("color by"), lasview.COLORINGS, 0))
        self.addParameter(ParameterSelection(lasview.SIZE,
            self.tr("window size (x y) in pixels"), lasview.SIZES, 0))
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasview")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        points = self.getParameterValue(lasview.POINTS)
        commands.append("-points " + str(points))
        coloring = self.getParameterValue(lasview.COLORING)
        if coloring != 0:
            commands.append("-color_by_" + lasview.COLORINGS[coloring])
        size = self.getParameterValue(lasview.SIZE)
        if size != 0:
            commands.append("-win " + lasview.SIZES[size])
        self.addParametersAdditionalCommands(commands)

        print commands
        LAStoolsUtils.runLAStools(commands, progress)
