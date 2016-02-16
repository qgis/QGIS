# -*- coding: utf-8 -*-

"""
***************************************************************************
    lassortPro.py
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

from processing.core.parameters import ParameterBoolean


class lassortPro(LAStoolsAlgorithm):

    BY_GPS_TIME = "BY_GPS_TIME"
    BY_POINT_SOURCE_ID = "BY_POINT_SOURCE_ID"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lassortPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterBoolean(lassortPro.BY_GPS_TIME,
                                           self.tr("sort by GPS time"), False))
        self.addParameter(ParameterBoolean(lassortPro.BY_POINT_SOURCE_ID,
                                           self.tr("sort by point source ID"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lassort")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        if self.getParameterValue(lassortPro.BY_GPS_TIME):
            commands.append("-gps_time")
        if self.getParameterValue(lassortPro.BY_POINT_SOURCE_ID):
            commands.append("-point_source")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
