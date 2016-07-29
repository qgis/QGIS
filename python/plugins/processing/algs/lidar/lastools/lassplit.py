# -*- coding: utf-8 -*-

"""
***************************************************************************
    lassplit.py
    ---------------------
    Date                 : March 2014
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
__date__ = 'March 2014'
__copyright__ = '(C) 2014, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection


class lassplit(LAStoolsAlgorithm):

    DIGITS = "DIGITS"
    OPERATION = "OPERATION"
    OPERATIONS = ["by_flightline", "by_classification", "by_gps_time_interval", "by_intensity_interval", "by_x_interval", "by_y_interval", "by_z_interval", "by_scan_angle_interval", "by_user_data_interval", "every_x_points", "recover_flightlines"]
    INTERVAL = "INTERVAL"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lassplit')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(lassplit.DIGITS,
                                          self.tr("number of digits for file names"), 0, None, 5))
        self.addParameter(ParameterSelection(lassplit.OPERATION,
                                             self.tr("how to split"), lassplit.OPERATIONS, 0))
        self.addParameter(ParameterNumber(lassplit.INTERVAL,
                                          self.tr("interval or number"), 0, None, 5))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lassplit")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        digits = self.getParameterValue(lassplit.DIGITS)
        if digits != 5:
            commands.append("-digits")
            commands.append(unicode(digits))
        operation = self.getParameterValue(lassplit.OPERATION)
        if operation != 0:
            if operation == 9:
                commands.append("-split")
            else:
                commands.append("-" + lassplit.OPERATIONS[operation])
        if operation > 1 and operation < 10:
            interval = self.getParameterValue(lassplit.INTERVAL)
            commands.append(unicode(interval))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
