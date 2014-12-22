# -*- coding: utf-8 -*-

"""
***************************************************************************
    laszip.py
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

class laszip(LAStoolsAlgorithm):

    REPORT_SIZE = "REPORT_SIZE"
    CREATE_LAX = "CREATE_LAX"
    APPEND_LAX = "APPEND_LAX"

    def defineCharacteristics(self):
        self.name = "laszip"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(laszip.REPORT_SIZE, "only report size", False))
        self.addParameter(ParameterBoolean(laszip.CREATE_LAX, "create spatial indexing file (*.lax)", False))
        self.addParameter(ParameterBoolean(laszip.APPEND_LAX, "append *.lax into *.laz file", False))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()


    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "laszip")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(laszip.REPORT_SIZE):
            commands.append("-size")
        if self.getParameterValue(laszip.CREATE_LAX):
            commands.append("-lax")
        if self.getParameterValue(laszip.APPEND_LAX):
            commands.append("-append")
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
