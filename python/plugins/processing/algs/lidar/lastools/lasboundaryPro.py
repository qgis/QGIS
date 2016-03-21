# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasboundaryPro.py
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


class lasboundaryPro(LAStoolsAlgorithm):

    MODE = "MODE"
    MODES = ["points", "spatial index (the *.lax file)", "bounding box"]
    CONCAVITY = "CONCAVITY"
    DISJOINT = "DISJOINT"
    HOLES = "HOLES"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasboundaryPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParameter(ParameterSelection(lasboundaryPro.MODE,
                                             self.tr("compute boundary based on"), lasboundaryPro.MODES, 0))
        self.addParameter(ParameterNumber(lasboundaryPro.CONCAVITY,
                                          self.tr("concavity"), 0, None, 50.0))
        self.addParameter(ParameterBoolean(lasboundaryPro.HOLES,
                                           self.tr("interior holes"), False))
        self.addParameter(ParameterBoolean(lasboundaryPro.DISJOINT,
                                           self.tr("disjoint polygon"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersVectorOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasboundary")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        mode = self.getParameterValue(lasboundaryPro.MODE)
        if (mode != 0):
            if (mode == 1):
                commands.append("-use_lax")
            else:
                commands.append("-use_bb")
        else:
            concavity = self.getParameterValue(lasboundaryPro.CONCAVITY)
            commands.append("-concavity")
            commands.append(unicode(concavity))
            if self.getParameterValue(lasboundaryPro.HOLES):
                commands.append("-holes")
            if self.getParameterValue(lasboundaryPro.DISJOINT):
                commands.append("-disjoint")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersVectorOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
