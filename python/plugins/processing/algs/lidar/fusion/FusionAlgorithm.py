# -*- coding: utf-8 -*-

"""
***************************************************************************
    FusionAlgorithm.py
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
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterString
from FusionUtils import FusionUtils


class FusionAlgorithm(GeoAlgorithm):

    ADVANCED_MODIFIERS = 'ADVANCED_MODIFIERS'

    def getIcon(self):
        filepath = os.path.dirname(__file__) + '/../../../images/tool.png'
        return QtGui.QIcon(filepath)

    def checkBeforeOpeningParametersDialog(self):
        path = FusionUtils.FusionPath()
        if path == '':
            return 'Fusion folder is not configured.\nPlease configure it \
                    before running Fusion algorithms.'

    def addAdvancedModifiers(self):
        param = ParameterString(self.ADVANCED_MODIFIERS, 'Additional modifiers'
                                , '')
        param.isAdvanced = True
        self.addParameter(param)

    def addAdvancedModifiersToCommand(self, commands):
        s = str(self.getParameterValue(self.ADVANCED_MODIFIERS)).strip()
        if s != '':
            commands.append(s)
