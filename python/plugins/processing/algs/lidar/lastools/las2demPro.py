# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2demPro.py
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

class las2demPro(LAStoolsAlgorithm):

    ATTRIBUTE = "ATTRIBUTE"
    PRODUCT = "PRODUCT"
    ATTRIBUTES = ["elevation", "slope", "intensity", "rgb", "edge_longest", "edge_shortest"]
    PRODUCTS = ["actual values", "hillshade", "gray", "false"]
    USE_TILE_BB = "USE_TILE_BB"

    def defineCharacteristics(self):
        self.name = "las2demPro"
        self.group = "LAStools"
        self.addParametersPointInputFolderGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParametersStepGUI()
        self.addParameter(ParameterSelection(las2demPro.ATTRIBUTE,
            self.tr("attribute (what to interpolate)"), las2demPro.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(las2demPro.PRODUCT,
            self.tr("product (how to output per pixel)"), las2demPro.PRODUCTS, 0))
        self.addParameter(ParameterBoolean(las2demPro.USE_TILE_BB,
            self.tr("use tile bounding box (after tiling with buffer)"), False))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersRasterOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2dem")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        self.addParametersStepCommands(commands)
        attribute = self.getParameterValue(las2demPro.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + las2demPro.ATTRIBUTES[attribute])
        product = self.getParameterValue(las2demPro.PRODUCT)
        if product != 0:
            commands.append("-" + las2demPro.PRODUCTS[product])
        if (self.getParameterValue(las2demPro.USE_TILE_BB)):
            commands.append("-use_tile_bb")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
