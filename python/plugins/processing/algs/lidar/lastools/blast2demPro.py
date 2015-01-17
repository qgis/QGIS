# -*- coding: utf-8 -*-

"""
***************************************************************************
    blast2demPro.py
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

class blast2demPro(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT = "PRODUCT"
    ATTRIBUTES = ["elevation", "slope", "intensity", "rgb"]
    PRODUCTS = ["actual values", "hillshade", "gray", "false"]
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name = "blast2demPro"
        self.group = "LAStools Production"
        self.addParametersPointInputFolderGUI()
        self.addParametersPointInputMergedGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(blast2demPro.ATTRIBUTE,
            self.tr("Attribute"), blast2demPro.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(blast2demPro.PRODUCT,
            self.tr("Product"), blast2demPro.PRODUCTS, 0))
        self.addParameter(ParameterBoolean(blast2demPro.USE_TILE_BB,
            self.tr("Use tile bounding box (after tiling with buffer)"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersRasterOutputFormatGUI()
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "blast2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersPointInputMergedCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersStepCommands(commands)
        attribute = self.getParameterValue(blast2demPro.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + blast2demPro.ATTRIBUTES[attribute])
        product = self.getParameterValue(blast2demPro.PRODUCT)
        if product != 0:
            commands.append("-" + blast2demPro.PRODUCTS[product])
        if (self.getParameterValue(blast2demPro.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
