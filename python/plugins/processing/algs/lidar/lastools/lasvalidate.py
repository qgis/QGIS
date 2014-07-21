# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasvalidate.py
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
from processing.core.outputs import OutputFile

class lasvalidate(LAStoolsAlgorithm):

    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasvalidate"
        self.group = "LAStools"
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addOutput(OutputFile(lasvalidate.OUTPUT, "Output XML file"))

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasvalidate.exe")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        commands.append("-o")
        commands.append(self.getOutputValue(lasvalidate.OUTPUT))

        LAStoolsUtils.runLAStools(commands, progress)
