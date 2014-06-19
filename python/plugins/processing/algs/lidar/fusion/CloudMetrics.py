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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from processing.parameters.ParameterFile import ParameterFile
from processing.outputs.OutputFile import OutputFile
from processing.lidar.fusion.FusionUtils import FusionUtils
from processing.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterBoolean import ParameterBoolean


class CloudMetrics(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    ABOVE = 'ABOVE'
    FIRSTINPULSE = 'FIRSTINPULSE'
    FIRSTRETURN = 'FIRSTRETURN'
    HTMIN = 'HTMIN'

    def defineCharacteristics(self):
        self.name = 'Cloud Metrics'
        self.group = 'Points'
        self.addParameter(ParameterFile(self.INPUT, 'Input las layer'))
        self.addOutput(OutputFile(self.OUTPUT, 'Output file with tabular metric information'))
        above = ParameterString(self.ABOVE, 'Above', '', False)
        above.isAdvanced = True
        self.addParameter(above)
        first_inpulse = ParameterBoolean(self.FIRSTINPULSE, 'First Inpulse', 0)
        first_inpulse.isAdvanced = True
        self.addParameter(first_inpulse)
        first_return = ParameterBoolean(self.FIRSTRETURN, 'First Return', 0)
        first_return.isAdvanced = True
        self.addParameter(first_return)
        htmin = ParameterString(self.HTMIN, 'Htmin', '', False, True)
        htmin.isAdvanced = True
        self.addParameter(htmin)

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'CloudMetrics.exe')]
        commands.append('/verbose')
        above = self.getParameterValue(self.ABOVE)
        if str(above).strip() != '':
            commands.append('/above:' + str(above))
        first_inpulse = self.getParameterValue(self.FIRSTINPULSE)
        if first_inpulse == 1:
            commands.append('/firstinpulse:' + first_inpulse)
        first_return = self.getParameterValue(self.FIRSTRETURN)
        if first_return == 1:
            commands.append('/firstreturn:' + first_return)
        htmin = self.getParameterValue(self.HTMIN)
        if str(htmin).strip() != '':
            commands.append('/minht:' + str(htmin))
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT) + ".dtm")
        FusionUtils.runFusion(commands, progress)
