# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2dem.py
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

class las2dem(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT = "PRODUCT"
    ATTRIBUTES = ["elevation", "slope", "intensity", "rgb", "edge_longest", "edge_shortest"]
    PRODUCTS = ["actual values", "hillshade", "gray", "false"]
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('las2dem')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(las2dem.ATTRIBUTE,
            self.tr("Attribute"), las2dem.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(las2dem.PRODUCT,
            self.tr("Product"), las2dem.PRODUCTS, 0))
        self.addParameter(ParameterBoolean(las2dem.USE_TILE_BB,
            self.tr("use tile bounding box (after tiling with buffer)"), False))
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersStepCommands(commands)
        attribute = self.getParameterValue(las2dem.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + las2dem.ATTRIBUTES[attribute])
        product = self.getParameterValue(las2dem.PRODUCT)
        if product != 0:
            commands.append("-" + las2dem.PRODUCTS[product])
        if (self.getParameterValue(las2dem.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
