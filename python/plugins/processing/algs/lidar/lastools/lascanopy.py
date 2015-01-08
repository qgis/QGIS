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

    PLOT_SIZE = "PLOT_SIZE"
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
    PRODUCTS = ["---", "min", "max", "avg", "std", "ske", "kur", "qav", "cov", "dns", "all",
                "p 1", "p 5", "p 10", "p 25", "p 50", "p 75", "p 90", "p 99",
                "int_min", "int_max", "int_avg", "int_std", "int_ske", "int_kur",
                "int_p 1", "int_p 5", "int_p 10", "int_p 25", "int_p 50", "int_p 75", "int_p 90", "int_p 99"]
    COUNTS = "COUNTS"
    DENSITIES = "DENSITIES"
    USE_TILE_BB = "USE_TILE_BB"
    FILES_ARE_PLOTS = "FILES_ARE_PLOTS"

    def defineCharacteristics(self):
        self.name = "lascanopy"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(lascanopy.PLOT_SIZE, "square plot size", 0, None, 20))
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
        self.addParameter(ParameterBoolean(lascanopy.FILES_ARE_PLOTS, "input file is single plot", False))
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lascanopy")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        plot_size = self.getParameterValue(lascanopy.PLOT_SIZE)
        if plot_size != 20:
            commands.append("-step")
            commands.append(str(plot_size))
        height_cutoff = self.getParameterValue(lascanopy.HEIGHT_CUTOFF)
        if height_cutoff != 1.37:
            commands.append("-height_cutoff")
            commands.append(str(height_cutoff))
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
        array = self.getParameterValue(lascanopy.COUNTS).split()
        if (len(array) > 1):
            commands.append("-c")
            for a in array:
                commands.append(a)
        array = self.getParameterValue(lascanopy.DENSITIES).split()
        if (len(array) > 1):
            commands.append("-d")
            for a in array:
                commands.append(a)
        if (self.getParameterValue(lascanopy.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        if (self.getParameterValue(lascanopy.FILES_ARE_PLOTS)):
            commands.append("-files_are_plots")
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
