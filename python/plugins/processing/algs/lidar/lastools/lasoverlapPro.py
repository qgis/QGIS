# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasoverlapPro.py
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
from processing.core.parameters import ParameterSelection


class lasoverlapPro(LAStoolsAlgorithm):

    CHECK_STEP = "CHECK_STEP"
    ATTRIBUTE = "ATTRIBUTE"
    OPERATION = "OPERATION"
    ATTRIBUTES = ["elevation", "intensity", "number_of_returns", "scan_angle_abs", "density"]
    OPERATIONS = ["lowest", "highest", "average"]
    CREATE_OVERLAP_RASTER = "CREATE_OVERLAP_RASTER"
    CREATE_DIFFERENCE_RASTER = "CREATE_DIFFERENCE_RASTER"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasoverlapPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParametersFilesAreFlightlinesGUI()
        self.addParametersFilter1ReturnClassFlagsGUI()
        self.addParameter(ParameterNumber(lasoverlapPro.CHECK_STEP,
                                          self.tr("size of grid used for overlap check"), 0, None, 2.0))
        self.addParameter(ParameterSelection(lasoverlapPro.ATTRIBUTE,
                                             self.tr("attribute to check"), lasoverlapPro.ATTRIBUTES, 0))
        self.addParameter(ParameterSelection(lasoverlapPro.OPERATION,
                                             self.tr("operation on attribute per cell"), lasoverlapPro.OPERATIONS, 0))
        self.addParameter(ParameterBoolean(lasoverlapPro.CREATE_OVERLAP_RASTER,
                                           self.tr("create overlap raster"), True))
        self.addParameter(ParameterBoolean(lasoverlapPro.CREATE_DIFFERENCE_RASTER,
                                           self.tr("create difference raster"), True))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersRasterOutputFormatGUI()
        self.addParametersRasterOutputGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasoverlap")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        self.addParametersFilesAreFlightlinesCommands(commands)
        self.addParametersFilter1ReturnClassFlagsCommands(commands)
        step = self.getParameterValue(lasoverlapPro.CHECK_STEP)
        if step != 0.0:
            commands.append("-step")
            commands.append(unicode(step))
        commands.append("-values")
        attribute = self.getParameterValue(lasoverlapPro.ATTRIBUTE)
        if attribute != 0:
            commands.append("-" + lasoverlapPro.ATTRIBUTES[attribute])
        operation = self.getParameterValue(lasoverlapPro.OPERATION)
        if operation != 0:
            commands.append("-" + lasoverlapPro.OPERATIONS[operation])
        if not self.getParameterValue(lasoverlapPro.CREATE_OVERLAP_RASTER):
            commands.append("-no_over")
        if not self.getParameterValue(lasoverlapPro.CREATE_DIFFERENCE_RASTER):
            commands.append("-no_diff")
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersRasterOutputFormatCommands(commands)
        self.addParametersRasterOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
