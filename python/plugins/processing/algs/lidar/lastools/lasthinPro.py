# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasthinPro.py
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


class lasthinPro(LAStoolsAlgorithm):

    THIN_STEP = "THIN_STEP"
    OPERATION = "OPERATION"
    OPERATIONS = ["lowest", "random", "highest"]
    WITHHELD = "WITHHELD"
    CLASSIFY_AS = "CLASSIFY_AS"
    CLASSIFY_AS_CLASS = "CLASSIFY_AS_CLASS"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasthinPro')
        self.group, self.i18n_group = self.trAlgorithm('LAStools Production')
        self.addParametersPointInputFolderGUI()
        self.addParameter(ParameterNumber(lasthinPro.THIN_STEP,
                                          self.tr("size of grid used for thinning"), 0, None, 1.0))
        self.addParameter(ParameterSelection(lasthinPro.OPERATION,
                                             self.tr("keep particular point per cell"), lasthinPro.OPERATIONS, 0))
        self.addParameter(ParameterBoolean(lasthinPro.WITHHELD,
                                           self.tr("mark thinned-away points as withheld"), False))
        self.addParameter(ParameterBoolean(lasthinPro.CLASSIFY_AS,
                                           self.tr("classify surviving points as class"), False))
        self.addParameter(ParameterNumber(lasthinPro.CLASSIFY_AS_CLASS,
                                          self.tr("class"), 0, None, 8))
        self.addParametersOutputDirectoryGUI()
        self.addParametersOutputAppendixGUI()
        self.addParametersPointOutputFormatGUI()
        self.addParametersAdditionalGUI()
        self.addParametersCoresGUI()
        self.addParametersVerboseGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasthin")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputFolderCommands(commands)
        step = self.getParameterValue(lasthinPro.THIN_STEP)
        if step != 0.0:
            commands.append("-step")
            commands.append(unicode(step))
        operation = self.getParameterValue(lasthinPro.OPERATION)
        if operation != 0:
            commands.append("-" + self.OPERATIONS[operation])
        if self.getParameterValue(lasthinPro.WITHHELD):
            commands.append("-withheld")
        if self.getParameterValue(lasthinPro.CLASSIFY_AS):
            commands.append("-classify_as")
            commands.append(unicode(self.getParameterValue(lasthinPro.CLASSIFY_AS_CLASS)))
        self.addParametersOutputDirectoryCommands(commands)
        self.addParametersOutputAppendixCommands(commands)
        self.addParametersPointOutputFormatCommands(commands)
        self.addParametersAdditionalCommands(commands)
        self.addParametersCoresCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
