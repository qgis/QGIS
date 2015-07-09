# -*- coding: utf-8 -*-

"""
***************************************************************************
    lassort.py
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

from processing.core.parameters import ParameterBoolean

class lassort(LAStoolsAlgorithm):

    BY_GPS_TIME = "BY_GPS_TIME"
    BY_POINT_SOURCE_ID = "BY_POINT_SOURCE_ID"

    def defineCharacteristics(self):
        self.name = "lassort"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(lassort.BY_GPS_TIME,
            self.tr("sort by GPS time"), False))
        self.addParameter(ParameterBoolean(lassort.BY_POINT_SOURCE_ID,
            self.tr("sort by point source ID"), False))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lassort")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(lassort.BY_GPS_TIME):
            commands.append("-gps_time")
        if self.getParameterValue(lassort.BY_POINT_SOURCE_ID):
            commands.append("-point_source")
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
