# -*- coding: utf-8 -*-

"""
***************************************************************************
    las2las_transform.py
    ---------------------
    Date                 : September 2013 and May 2016
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
from .LAStoolsUtils import LAStoolsUtils
from .LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection


class las2las_transform(LAStoolsAlgorithm):

    OPERATION = "OPERATION"
    OPERATIONS = ["---", "set_point_type", "set_point_size", "set_version_minor", "set_version_major", "start_at_point", "stop_at_point", "remove_vlr", "week_to_adjusted", "adjusted_to_week", "auto_reoffset", "scale_rgb_up", "scale_rgb_down", "remove_all_vlrs", "remove_extra", "clip_to_bounding_box"]
    OPERATIONARG = "OPERATIONARG"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('las2las_transform')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersTransform1CoordinateGUI()
        self.addParametersTransform2CoordinateGUI()
        self.addParametersTransform1OtherGUI()
        self.addParametersTransform2OtherGUI()
        self.addParameter(ParameterSelection(las2las_transform.OPERATION,
                                             self.tr("operations (first 8 need an argument)"), las2las_transform.OPERATIONS, 0))
        self.addParameter(ParameterString(las2las_transform.OPERATIONARG,
                                          self.tr("argument for operation")))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        if (LAStoolsUtils.hasWine()):
            commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2las.exe")]
        else:
            commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "las2las")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersTransform1CoordinateCommands(commands)
        self.addParametersTransform2CoordinateCommands(commands)
        self.addParametersTransform1OtherCommands(commands)
        self.addParametersTransform2OtherCommands(commands)
        operation = self.getParameterValue(las2las_transform.OPERATION)
        if operation != 0:
            commands.append("-" + las2las_transform.OPERATIONS[operation])
            if operation > 8:
                commands.append(self.getParameterValue(las2las_transform.OPERATIONARG))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
