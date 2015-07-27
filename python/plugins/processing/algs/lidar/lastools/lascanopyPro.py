# -*- coding: utf-8 -*-

"""
***************************************************************************
    lascanopyPro.py
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

from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection

class lascanopyPro(LAStoolsAlgorithm):

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
        self.name, self.i18n_name = self.trAlgorithm('lascanopyPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParametersPointInputMergedGUI()
        self.addParameter(ParameterNumber(lascanopyPro.PLOT_SIZE,
            self.tr("square plot size"), 0, None, 20))
        self.addParameter(ParameterNumber(lascanopyPro.HEIGHT_CUTOFF,
            self.tr("height cutoff / breast height"), 0, None, 1.37))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT1,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT2,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT3,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT4,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT5,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT6,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT7,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT8,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterSelection(lascanopyPro.PRODUCT9,
            self.tr("create"), lascanopyPro.PRODUCTS, 0))
        self.addParameter(ParameterString(lascanopyPro.COUNTS,
            self.tr("count rasters (e.g. 2.0 5.0 10.0 20.0)"), ""))
        self.addParameter(ParameterString(lascanopyPro.DENSITIES,
            self.tr("density rasters (e.g. 2.0 5.0 10.0 20.0)"), ""))
        self.addParameter(ParameterBoolean(lascanopyPro.USE_TILE_BB,
            self.tr("use tile bounding box (after tiling with buffer)"), False))
        self.addParameter(ParameterBoolean(lascanopyPro.FILES_ARE_PLOTS,
            self.tr("input file is single plot"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersRasterOutputFormatGUI()
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lascanopy")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersPointInputMergedCommands(commands)
        plot_size = self.getParameterValue(lascanopyPro.PLOT_SIZE)
        if plot_size != 20:
            commands.append("-step")
            commands.append(str(plot_size))
        height_cutoff = self.getParameterValue(lascanopyPro.HEIGHT_CUTOFF)
        if height_cutoff != 1.37:
            commands.append("-height_cutoff")
            commands.append(str(height_cutoff))
        product = self.getParameterValue(lascanopyPro.PRODUCT1)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT2)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT3)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT4)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT5)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT6)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT7)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT8)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        product = self.getParameterValue(lascanopyPro.PRODUCT9)
        if product != 0:
            commands.append("-" + lascanopyPro.PRODUCTS[product])
        array = self.getParameterValue(lascanopyPro.COUNTS).split()
        if (len(array) > 1):
            commands.append("-c")
            for a in array:
                commands.append(a)
        array = self.getParameterValue(lascanopyPro.DENSITIES).split()
        if (len(array) > 1):
            commands.append("-d")
            for a in array:
                commands.append(a)
        if (self.getParameterValue(lascanopyPro.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        if (self.getParameterValue(lascanopyPro.FILES_ARE_PLOTS)):
            commands.append("-files_are_plots")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
