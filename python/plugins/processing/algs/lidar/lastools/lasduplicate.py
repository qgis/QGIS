# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasduplicate.py
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
from processing.core.parameters import ParameterFile


class lasduplicate(LAStoolsAlgorithm):

    LOWEST_Z = "LOWEST_Z"
    UNIQUE_XYZ = "UNIQUE_XYZ"
    SINGLE_RETURNS = "SINGLE_RETURNS"
    RECORD_REMOVED = "RECORD_REMOVED"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasduplicate')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterBoolean(lasduplicate.LOWEST_Z,
                                           self.tr("keep duplicate with lowest z coordinate"), False))
        self.addParameter(ParameterBoolean(lasduplicate.UNIQUE_XYZ,
                                           self.tr("only remove duplicates in x y and z"), False))
        self.addParameter(ParameterBoolean(lasduplicate.SINGLE_RETURNS,
                                           self.tr("mark surviving duplicate as single return"), False))
        self.addParameter(ParameterFile(lasduplicate.RECORD_REMOVED,
                                        self.tr("record removed duplicates to LAS/LAZ file")))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasduplicate")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        if self.getParameterValue(lasduplicate.LOWEST_Z):
            commands.append("-lowest_z")
        if self.getParameterValue(lasduplicate.UNIQUE_XYZ):
            commands.append("-unique_xyz")
        if self.getParameterValue(lasduplicate.SINGLE_RETURNS):
            commands.append("-single_returns")
        record_removed = self.getParameterValue(lasduplicate.RECORD_REMOVED)
        if record_removed is not None and record_removed != "":
            commands.append("-record_removed")
            commands.append(record_removed)
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
