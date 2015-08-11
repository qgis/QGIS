# -*- coding: utf-8 -*-

"""
***************************************************************************
    shp2las.py
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

from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterFile

class shp2las(LAStoolsAlgorithm):

    INPUT = "INPUT"
    SCALE_FACTOR_XY = "SCALE_FACTOR_XY"
    SCALE_FACTOR_Z = "SCALE_FACTOR_Z"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('shp2las')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParameter(ParameterFile(shp2las.INPUT,
            self.tr("Input SHP file")))
        self.addParameter(ParameterNumber(shp2las.SCALE_FACTOR_XY,
            self.tr("resolution of x and y coordinate"), 0, None, 0.01))
        self.addParameter(ParameterNumber(shp2las.SCALE_FACTOR_Z,
            self.tr("resolution of z coordinate"), 0, None, 0.01))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "shp2las")]
        self.addParametersVerboseCommands(commands)
        commands.append("-i")
        commands.append(self.getParameterValue(shp2las.INPUT))
        scale_factor_xy = self.getParameterValue(shp2las.SCALE_FACTOR_XY)
        scale_factor_z = self.getParameterValue(shp2las.SCALE_FACTOR_Z)
        if scale_factor_xy != 0.01 or scale_factor_z != 0.01:
            commands.append("-set_scale_factor")
            commands.append(str(scale_factor_xy) + " " + str(scale_factor_xy) + " " + str(scale_factor_z))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
