# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasboundary.py
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber

class lasboundary(LAStoolsAlgorithm):

    MODE = "MODE"
    MODES = ["points", "spatial index (the *.lax file)", "bounding box"]
    CONCAVITY = "CONCAVITY"
    DISJOINT = "DISJOINT"
    HOLES = "HOLES"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasboundary')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParameter(ParameterSelection(lasboundary.MODE,
            self.tr("compute boundary based on"), lasboundary.MODES, 0))
        self.addParameter(ParameterNumber(lasboundary.CONCAVITY,
            self.tr("concavity"), 0, None, 50.0))
        self.addParameter(ParameterBoolean(lasboundary.HOLES,
            self.tr("interior holes"), False))
        self.addParameter(ParameterBoolean(lasboundary.DISJOINT,
            self.tr("disjoint polygon"), False))
        self.addParametersVectorOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasboundary")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        mode = self.getParameterValue(lasboundary.MODE)
        if (mode != 0):
            if (mode == 1):
                commands.append("-use_lax")
            else:
                commands.append("-use_bb")
        else:
            concavity = self.getParameterValue(lasboundary.CONCAVITY)
            commands.append("-concavity")
            commands.append(str(concavity))
            if self.getParameterValue(lasboundary.HOLES):
                commands.append("-holes")
            if self.getParameterValue(lasboundary.DISJOINT):
                commands.append("-disjoint")
        self.addParametersVectorOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
