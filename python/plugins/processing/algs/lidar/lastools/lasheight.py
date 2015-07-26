# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasheight.py
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
from processing.core.parameters import ParameterNumber

class lasheight(LAStoolsAlgorithm):

    REPLACE_Z = "REPLACE_Z"
    DROP_ABOVE = "DROP_ABOVE"
    DROP_ABOVE_HEIGHT = "DROP_ABOVE_HEIGHT"
    DROP_BELOW = "DROP_BELOW"
    DROP_BELOW_HEIGHT = "DROP_BELOW_HEIGHT"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasheight')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(lasheight.REPLACE_Z,
            self.tr("replace z"), False))
        self.addParameter(ParameterBoolean(lasheight.DROP_ABOVE,
            self.tr("drop above"), False))
        self.addParameter(ParameterNumber(lasheight.DROP_ABOVE_HEIGHT,
            self.tr("drop above height"), 0, None, 100.0))
        self.addParameter(ParameterBoolean(lasheight.DROP_BELOW,
            self.tr("drop below"), False))
        self.addParameter(ParameterNumber(lasheight.DROP_BELOW_HEIGHT,
            self.tr("drop below height"), 0, None, -2.0))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(lasheight.REPLACE_Z):
            commands.append("-replace_z")
        if self.getParameterValue(lasheight.DROP_ABOVE):
            commands.append("-drop_above")
            commands.append(str(self.getParameterValue(lasheight.DROP_ABOVE_HEIGHT)))
        if self.getParameterValue(lasheight.DROP_BELOW):
            commands.append("-drop_below")
            commands.append(str(self.getParameterValue(lasheight.DROP_BELOW_HEIGHT)))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
