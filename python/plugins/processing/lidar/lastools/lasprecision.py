# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasprecision.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
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
from PyQt4 import QtGui
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from processing.parameters.ParameterFile import ParameterFile
from processing.outputs.OutputHTML import OutputHTML

class lasprecision(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasprecision"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasprecision.INPUT, "Input las layer"))
        self.addOutput(OutputHTML(lasprecision.OUTPUT, "Output info file"))

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasprecision.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasprecision.INPUT))
        commands.append(">")
        commands.append(self.getOutputValue(lasprecision.OUTPUT) + ".txt")

        LasToolsUtils.runLasTools(commands, progress)
        fin = open (self.getOutputValue(lasprecision.OUTPUT) + ".txt")
        fout = open (self.getOutputValue(lasprecision.OUTPUT), "w")
        lines = fin.readlines()
        for line in lines:
            fout.write(line + "<br>")
        fin.close()
        fout.close()

