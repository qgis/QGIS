# -*- coding: utf-8 -*-

"""
***************************************************************************
    CloudMetrics.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Agresta S. Coop.
    Email                : iescamochero at agresta dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from future import standard_library
standard_library.install_aliases()
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputFile
from .FusionUtils import FusionUtils
from .FusionAlgorithm import FusionAlgorithm


class CloudMetrics(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    ABOVE = 'ABOVE'
    FIRSTIMPULSE = 'FIRSTIMPULSE'
    FIRSTRETURN = 'FIRSTRETURN'
    HTMIN = 'HTMIN'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Cloud Metrics')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer'),
            optional=False))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output file with tabular metric information'), 'csv'))
        above = ParameterNumber(self.ABOVE, self.tr('Compute cover statistics above the following heightbreak:'), 0, None, 0.0)
        above.isAdvanced = True
        self.addParameter(above)
        firstImpulse = ParameterBoolean(
            self.FIRSTIMPULSE, self.tr('First Impulse'), False)
        firstImpulse.isAdvanced = True
        self.addParameter(firstImpulse)
        firstReturn = ParameterBoolean(
            self.FIRSTRETURN, self.tr('First Return'), False)
        firstReturn.isAdvanced = True
        self.addParameter(firstReturn)
        htmin = ParameterNumber(self.HTMIN, self.tr('Use only returns above this minimum height:'), 0, None, 0)
        htmin.isAdvanced = True
        self.addParameter(htmin)

    def processAlgorithm(self, feedback):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CloudMetrics.exe')]
        commands.append('/verbose')
        above = self.getParameterValue(self.ABOVE)
        if above != 0.0:
            commands.append('/above:' + str(above))
        firstImpulse = self.getParameterValue(self.FIRSTIMPULSE)
        if firstImpulse:
            commands.append('/firstinpulse')
        firstReturn = self.getParameterValue(self.FIRSTRETURN)
        if firstReturn:
            commands.append('/firstreturn')
        htmin = self.getParameterValue(self.HTMIN)
        if htmin != 0.0:
            commands.append('/minht:' + str(htmin))
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        FusionUtils.runFusion(commands, feedback)
