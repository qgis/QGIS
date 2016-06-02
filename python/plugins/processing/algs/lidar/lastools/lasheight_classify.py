# -*- coding: utf-8 -*-

"""
***************************************************************************
    lasheight_classify.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Martin Isenburg
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Martin Isenburg'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from LAStoolsUtils import LAStoolsUtils
from LAStoolsAlgorithm import LAStoolsAlgorithm

from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection


class lasheight_classify(LAStoolsAlgorithm):

    REPLACE_Z = "REPLACE_Z"
    CLASSIFY_BELOW = "CLASSIFY_BELOW"
    CLASSIFY_BELOW_HEIGHT = "CLASSIFY_BELOW_HEIGHT"
    CLASSIFY_BETWEEN1 = "CLASSIFY_BETWEEN1"
    CLASSIFY_BETWEEN1_HEIGHT_FROM = "CLASSIFY_BETWEEN1_HEIGHT_FROM"
    CLASSIFY_BETWEEN1_HEIGHT_TO = "CLASSIFY_BETWEEN1_HEIGHT_TO"
    CLASSIFY_BETWEEN2 = "CLASSIFY_BETWEEN2"
    CLASSIFY_BETWEEN2_HEIGHT_FROM = "CLASSIFY_BETWEEN2_HEIGHT_FROM"
    CLASSIFY_BETWEEN2_HEIGHT_TO = "CLASSIFY_BETWEEN2_HEIGHT_TO"
    CLASSIFY_ABOVE = "CLASSIFY_ABOVE"
    CLASSIFY_ABOVE_HEIGHT = "CLASSIFY_ABOVE_HEIGHT"

    CLASSIFY_CLASSES = ["---", "unclassified (1)", "ground (2)", "veg low (3)", "veg mid (4)", "veg high (5)", "buildings (6)", "noise (7)", "keypoint (8)", "water (9)", "water (9)", "rail (10)", "road (11)", "overlap (12)"]

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('lasheight_classify')
        self.group, self.i18n_group = self.trAlgorithm('LAStools')
        self.addParametersVerboseGUI()
        self.addParametersPointInputGUI()
        self.addParametersIgnoreClass1GUI()
        self.addParametersIgnoreClass2GUI()
        self.addParameter(ParameterBoolean(lasheight_classify.REPLACE_Z,
                                           self.tr("replace z"), False))
        self.addParameter(ParameterSelection(lasheight_classify.CLASSIFY_BELOW,
                                             self.tr("classify below height as"), lasheight_classify.CLASSIFY_CLASSES, 0))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_BELOW_HEIGHT,
                                          self.tr("below height"), None, None, -2.0))
        self.addParameter(ParameterSelection(lasheight_classify.CLASSIFY_BETWEEN1,
                                             self.tr("classify between height as"), lasheight_classify.CLASSIFY_CLASSES, 0))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_BETWEEN1_HEIGHT_FROM,
                                          self.tr("between height ... "), None, None, 0.5))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_BETWEEN1_HEIGHT_TO,
                                          self.tr("... and height"), None, None, 2.0))
        self.addParameter(ParameterSelection(lasheight_classify.CLASSIFY_BETWEEN2,
                                             self.tr("classify between height as"), lasheight_classify.CLASSIFY_CLASSES, 0))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_BETWEEN2_HEIGHT_FROM,
                                          self.tr("between height ..."), None, None, 2.0))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_BETWEEN2_HEIGHT_TO,
                                          self.tr("... and height"), None, None, 5.0))
        self.addParameter(ParameterSelection(lasheight_classify.CLASSIFY_ABOVE,
                                             self.tr("classify above"), lasheight_classify.CLASSIFY_CLASSES, 0))
        self.addParameter(ParameterNumber(lasheight_classify.CLASSIFY_ABOVE_HEIGHT,
                                          self.tr("classify above height"), None, None, 100.0))
        self.addParametersPointOutputGUI()
        self.addParametersAdditionalGUI()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LAStoolsUtils.LAStoolsPath(), "bin", "lasheight")]
        self.addParametersVerboseCommands(commands)
        self.addParametersPointInputCommands(commands)
        self.addParametersIgnoreClass1Commands(commands)
        self.addParametersIgnoreClass2Commands(commands)
        if self.getParameterValue(lasheight_classify.REPLACE_Z):
            commands.append("-replace_z")
        classify = self.getParameterValue(lasheight_classify.CLASSIFY_BELOW)
        if (classify != 0):
            commands.append("-classify_below")
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_BELOW_HEIGHT)))
            commands.append(unicode(classify))
        classify = self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN1)
        if (classify != 0):
            commands.append("-classify_between")
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN1_HEIGHT_FROM)))
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN1_HEIGHT_TO)))
            commands.append(unicode(classify))
        classify = self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN2)
        if (classify != 0):
            commands.append("-classify_between")
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN2_HEIGHT_FROM)))
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_BETWEEN2_HEIGHT_TO)))
            commands.append(unicode(classify))
        classify = self.getParameterValue(lasheight_classify.CLASSIFY_ABOVE)
        if (classify != 0):
            commands.append("-classify_above")
            commands.append(unicode(self.getParameterValue(lasheight_classify.CLASSIFY_ABOVE_HEIGHT)))
            commands.append(unicode(classify))
        self.addParametersPointOutputCommands(commands)
        self.addParametersAdditionalCommands(commands)

        LAStoolsUtils.runLAStools(commands, progress)
