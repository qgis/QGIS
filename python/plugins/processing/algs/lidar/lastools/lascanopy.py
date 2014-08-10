# -*- coding: utf-8 -*-

"""
***************************************************************************
    lascanopy.py
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
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection

class lascanopy(LAStoolsAlgorithm):

    HEIGHT_CUTOFF = "HEIGHT_CUTOFF"
    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT1 = "PRODUCT1"
    PRODUCT2 = "PRODUCT2"
    PRODUCT3 = "PRODUCT3"
    PRODUCT4 = "PRODUCT4"
    PRODUCT5 = "PRODUCT5"
    PRODUCT6 = "PRODUCT6"
    PRODUCT7 = "PRODUCT7"
    PRODUCT8 = "PRODUCT8"
    PRODUCT9 = "PRODUCT9"
    PRODUCTS = ["---", "min", "max", "avg", "std", "ske", "kur", "cov", "dns",
                "p01", "p05", "p10", "p25", "p50", "p75", "p90", "p99"]
    COUNTS = "COUNTS"
    DENSITIES = "DENSITIES"
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name = "lascanopy"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterNumber(lascanopy.HEIGHT_CUTOFF, "height cutoff / breast height", 0, None, 1.37))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT1, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT2, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT3, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT4, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT5, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT6, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT7, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT8, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopy.PRODUCT9, "create", lascanopy.PRODUCTS, 0))
        self.addParameter(ParameterString(lascanopy.COUNTS, "count rasters (e.g. 2.0 5.0 10.0 20.0)", ""))
        self.addParameter(ParameterString(lascanopy.DENSITIES, "density rasters (e.g. 2.0 5.0 10.0 20.0)", ""))
        self.addParameter(ParameterBoolean(lascanopy.USE_TILE_BB, "use tile bounding box (after tiling with buffer)", False))
        self.addParametersRasterOutputGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lascanopy.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersStepCommands(commands)
        product = self.getParameterValue(lascanopy.PRODUCT1)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT2)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT3)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT4)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT5)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT6)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT7)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT8)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        product = self.getParameterValue(lascanopy.PRODUCT9)
        if product != 0:
            commands.append("-" + lascanopy.PRODUCTS[product])
        self.addParametersRasterOutputCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
