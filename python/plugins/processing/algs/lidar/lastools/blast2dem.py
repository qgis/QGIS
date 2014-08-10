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

class blast2dem(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT = "PRODUCT"
    ATTRIBUTES = ["elevation", "slope", "intensity", "rgb"]
    PRODUCTS = ["actual values", "hillshade", "gray", "false"]


    def defineCharacteristics(self):
        self.name = "blast2dem"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(blast2dem.ATTRIBUTE, "Attribute", blast2dem.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(blast2dem.PRODUCT, "Product", blast2dem.PRODUCTS, 0))
        self.addParametersRasterOutputGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "blast2dem.exe")]
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
        self.addParametersRasterOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
