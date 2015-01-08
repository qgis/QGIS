# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2lasPro_filter.py
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

class las2lasPro_filter(LAStoolsAlgorithm):

    def defineCharacteristics(self):
        self.name = "las2lasPro_filter"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersFilter2ReturnClassFlagsGUI()
        self.addParametersFilter1CoordsIntensityGUI()
        self.addParametersFilter2CoordsIntensityGUI()
        self.addParametersPointOutputGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2las")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersFilter2ReturnClassFlagsCommands(commands)
        self.addParametersFilter1CoordsIntensityCommands(commands)
        self.addParametersFilter2CoordsIntensityCommands(commands)
        self.addParametersPointOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
