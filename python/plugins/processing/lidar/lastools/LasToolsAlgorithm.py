# -*- coding: utf-8 -*-

"""
***************************************************************************
    LasToolsAlgorithm.py
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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterBoolean import ParameterBoolean
import os
from PyQt4 import QtGui
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils

class LasToolsAlgorithm(GeoAlgorithm):

    FIRST_ONLY = "FIRST_ONLY"
    LAST_ONLY = "LAST_ONLY"
    SINGLE_RET_ONLY = "SINGLE_RET_ONLY"
    DOUBLE_RET_ONLY = "DOUBLE_RET_ONLY"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/../images/tool.png"
        return QtGui.QIcon(filepath)

    def checkBeforeOpeningParametersDialog(self):
            path = LasToolsUtils.LasToolsPath()
            if path == "":
                return "LasTools folder is not configured.\nPlease configure it before running LasTools algorithms."

    def addCommonParameters(self):
        self.addParameter(ParameterBoolean(LasToolsAlgorithm.FIRST_ONLY, "Keep first return only", False))
        self.addParameter(ParameterBoolean(LasToolsAlgorithm.LAST_ONLY, "Keep last return only", False))
        self.addParameter(ParameterBoolean(LasToolsAlgorithm.SINGLE_RET_ONLY, "Keep single returns only", False))
        self.addParameter(ParameterBoolean(LasToolsAlgorithm.DOUBLE_RET_ONLY, "Keep double returns only", False))

    def addCommonParameterValuesToCommand(self, commands):
        if self.getParameterValue(LasToolsAlgorithm.LAST_ONLY):
            commands.append("-last_only")
        if self.getParameterValue(LasToolsAlgorithm.FIRST_ONLY):
            commands.append("-first_only")
        if self.getParameterValue(LasToolsAlgorithm.SINGLE_RET_ONLY):
            commands.append("-single_returns_only")
        if self.getParameterValue(LasToolsAlgorithm.DOUBLE_RET_ONLY):
            commands.append("-double_returns_only")
