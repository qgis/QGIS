# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipData.py
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
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputFile
from .FusionAlgorithm import FusionAlgorithm
from .FusionUtils import FusionUtils


class ClipData(FusionAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    EXTENT = 'EXTENT'
    SHAPE = 'SHAPE'
    DTM = 'DTM'
    HEIGHT = 'HEIGHT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip Data')
        self.group, self.i18n_group = self.trAlgorithm('Points')
        self.addParameter(ParameterFile(
            self.INPUT, self.tr('Input LAS layer')))
        self.addParameter(ParameterExtent(self.EXTENT, self.tr('Extent')))
        self.addParameter(ParameterSelection(
            self.SHAPE, self.tr('Shape'), ['Rectangle', 'Circle']))
        self.addOutput(OutputFile(
            self.OUTPUT, self.tr('Output clipped LAS file')))
        dtm = ParameterFile(
            self.DTM, self.tr('Ground file for height normalization'))
        dtm.isAdvanced = True
        self.addParameter(dtm)
        height = ParameterBoolean(
            self.HEIGHT, self.tr("Convert point elevations into heights above ground (used with the above command)"), False)
        height.isAdvanced = True
        self.addParameter(height)
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), 'ClipData.exe')]
        commands.append('/verbose')
        self.addAdvancedModifiersToCommand(commands)
        commands.append('/shape:' + unicode(self.getParameterValue(self.SHAPE)))
        dtm = self.getParameterValue(self.DTM)
        if dtm:
            commands.append('/dtm:' + unicode(dtm))
        height = self.getParameterValue(self.HEIGHT)
        if height:
            commands.append('/height')
        files = self.getParameterValue(self.INPUT).split(';')
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        extent = unicode(self.getParameterValue(self.EXTENT)).split(',')
        commands.append(extent[0])
        commands.append(extent[2])
        commands.append(extent[1])
        commands.append(extent[3])
        FusionUtils.runFusion(commands, progress)
