# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasthin.py
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
from processing.core.parameters import ParameterSelection


class lasthin(LAStoolsAlgorithm):

    THIN_STEP = "THIN_STEP"
    OPERATION = "OPERATION"
    OPERATIONS = ["lowest", "random", "highest"]
    WITHHELD = "WITHHELD"
    CLASSIFY_AS = "CLASSIFY_AS"
    CLASSIFY_AS_CLASS = "CLASSIFY_AS_CLASS"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasthin')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParameter(ParameterNumber(lasthin.THIN_STEP,
                                          self.tr("size of grid used for thinning"), 0, None, 1.0))
        self.addParameter(ParameterSelection(lasthin.OPERATION,
                                             self.tr("keep particular point per cell"), lasthin.OPERATIONS, 0))
        self.addParameter(ParameterBoolean(lasthin.WITHHELD,
                                           self.tr("mark thinned-away points as withheld"), False))
        self.addParameter(ParameterBoolean(lasthin.CLASSIFY_AS,
                                           self.tr("classify surviving points as class"), False))
        self.addParameter(ParameterNumber(lasthin.CLASSIFY_AS_CLASS,
                                          self.tr("class"), 0, None, 8))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasthin")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        step = self.getParameterValue(lasthin.THIN_STEP)
        if step != 0.0:
            commands.append("-step")
            commands.append(unicode(step))
        operation = self.getParameterValue(lasthin.OPERATION)
        if operation != 0:
            commands.append("-" + self.OPERATIONS[operation])
        if self.getParameterValue(lasthin.WITHHELD):
            commands.append("-withheld")
        if self.getParameterValue(lasthin.CLASSIFY_AS):
            commands.append("-classify_as")
            commands.append(unicode(self.getParameterValue(lasthin.CLASSIFY_AS_CLASS)))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
