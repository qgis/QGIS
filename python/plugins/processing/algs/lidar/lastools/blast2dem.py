# -*- coding: utf-8 -*-

"""
***************************************************************************
    blast2dem.py
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

from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterBoolean

class blast2dem(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT = "PRODUCT"
    ATTRIBUTES = ["elevation", "slope", "intensity", "rgb"]
    PRODUCTS = ["actual values", "hillshade", "gray", "false"]
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('blast2dem')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(blast2dem.ATTRIBUTE,
            self.tr("Attribute"), blast2dem.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(blast2dem.PRODUCT,
            self.tr("Product"), blast2dem.PRODUCTS, 0))
        self.addParameter(ParameterBoolean(blast2dem.USE_TILE_BB,
            self.tr("Use tile bounding box (after tiling with buffer)"), False))
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "blast2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersStepCommands(commands)
        attribute = self.getParameterValue(blast2dem.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + blast2dem.ATTRIBUTES[attribute])
        product = self.getParameterValue(blast2dem.PRODUCT)
        if product != 0:
            commands.append("-" + blast2dem.PRODUCTS[product])
        if (self.getParameterValue(blast2dem.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
