# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasgridPro.py
    ---------------------
    Date                 : October 2014
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean


class lasgridPro(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    METHOD = "METHOD"
    ATTRIBUTES = ["elevation", "intensity", "rgb", "classification"]
    METHODS = ["lowest", "highest", "average", "stddev"]
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasgridPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParametersPointInputMergedGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(lasgridPro.ATTRIBUTE,
                                             self.tr("Attribute"), lasgridPro.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(lasgridPro.METHOD,
                                             self.tr("Method"), lasgridPro.METHODS, 0))
        self.addParameter(ParameterBoolean(lasgridPro.USE_TILE_BB,
                                           self.tr("use tile bounding box (after tiling with buffer)"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersRasterOutputFormatGUI()
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasgrid")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersPointInputMergedCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersStepCommands(commands)
        attribute = self.getParameterValue(lasgridPro.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + lasgridPro.ATTRIBUTES[attribute])
        method = self.getParameterValue(lasgridPro.METHOD)
        if method != 0:
            commands.append("-" + lasgridPro.METHODS[method])
        if (self.getParameterValue(lasgridPro.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
