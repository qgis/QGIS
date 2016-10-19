# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrinfo.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean

from processing.core.outputs import OutputHTML

from processing.algs.gdal.GdalUtils import GdalUtils
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.tools import dataobjects
from processing.tools.vector import ogrConnectionString


class OgrInfo(GdalAlgorithm):

    INPUT = 'INPUT'
    SUMMARY_ONLY = 'SUMMARY_ONLY'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Information')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Miscellaneous')

        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer'),
                                          [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterBoolean(self.SUMMARY_ONLY,
                                           self.tr('Summary output only'),
                                           True))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Layer information')))

    def getConsoleCommands(self):
        arguments = ["ogrinfo"]
        arguments.append('-al')
        if self.getParameterValue(self.SUMMARY_ONLY):
            arguments.append('-so')
        layer = self.getParameterValue(self.INPUT)
        conn = ogrConnectionString(layer)
        arguments.append(conn)
        return arguments

    def processAlgorithm(self, progress):
        commands = self.getConsoleCommands()
        layers = dataobjects.getVectorLayers()
        supported = dataobjects.getSupportedOutputVectorLayerExtensions()
        for i, c in enumerate(commands):
            for layer in layers:
                if layer.source() in c:
                    if layer.dataProvider().name() == 'memory':
                        exported = dataobjects.exportVectorLayer(layer, supported, "sqlite")
                        exportedFileName = os.path.splitext(os.path.split(exported)[1])[0]
                        c = c.replace(layer.source(), exported)
                        if os.path.isfile(layer.source()):
                            fileName = os.path.splitext(os.path.split(layer.source())[1])[0]
                            c = re.sub('[\s]{}[\s]'.format(fileName), ' ' + exportedFileName + ' ', c)
                            c = re.sub('[\s]{}'.format(fileName), ' ' + exportedFileName, c)
                            c = re.sub('["\']{}["\']'.format(fileName), "'" + exportedFileName + "'", c)

            commands[i] = c
        GdalUtils.runGdal(commands, progress)
        output = self.getOutputValue(self.OUTPUT)
        with open(output, 'w') as f:
            f.write('<pre>')
            for s in GdalUtils.getConsoleOutput()[1:]:
                f.write(s)
            f.write('</pre>')
