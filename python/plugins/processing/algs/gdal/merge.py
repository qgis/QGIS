# -*- coding: utf-8 -*-

"""
***************************************************************************
    merge.py
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


from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.outputs import OutputRaster
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterSelection
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils


class merge(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    PCT = 'PCT'
    SEPARATE = 'SEPARATE'
    RTYPE = 'RTYPE'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def defineCharacteristics(self):
        self.name = 'Merge'
        self.group = '[GDAL] Miscellaneous'
        self.addParameter(ParameterMultipleInput(merge.INPUT,
            self.tr('Input layers'), ParameterMultipleInput.TYPE_RASTER))
        self.addParameter(ParameterBoolean(merge.PCT,
            self.tr('Grab pseudocolor table from first layer'), False))
        self.addParameter(ParameterBoolean(merge.SEPARATE,
            self.tr('Layer stack'), False))
        self.addParameter(ParameterSelection(self.RTYPE,
            self.tr('Output raster type'), self.TYPE, 5))

        self.addOutput(OutputRaster(merge.OUTPUT, self.tr('Merged')))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if self.getParameterValue(merge.SEPARATE):
            arguments.append('-separate')
        if self.getParameterValue(merge.PCT):
            arguments.append('-pct')
        arguments.append('-o')
        out = self.getOutputValue(merge.OUTPUT)
        arguments.append(out)
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.extend(self.getParameterValue(merge.INPUT).split(';'))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_merge.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_merge.py', GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)
