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

import os

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import (ParameterBoolean,
                                        ParameterString,
                                        ParameterSelection,
                                        ParameterMultipleInput)
from processing.core.outputs import OutputRaster
from processing.tools.system import isWindows
from processing.tools import dataobjects

from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class merge(GdalAlgorithm):

    INPUT = 'INPUT'
    OPTIONS = 'OPTIONS'
    PCT = 'PCT'
    SEPARATE = 'SEPARATE'
    RTYPE = 'RTYPE'
    OUTPUT = 'OUTPUT'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64']

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'merge.png'))

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterMultipleInput(self.INPUT,
                                                 self.tr('Input layers'),
                                                 dataobjects.TYPE_RASTER))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'),
                                          optional=True,
                                          metadata={'widget_wrapper': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}))
        self.addParameter(ParameterBoolean(self.PCT,
                                           self.tr('Grab pseudocolor table from first layer'),
                                           False))
        self.addParameter(ParameterBoolean(self.SEPARATE,
                                           self.tr('Place each input file into a separate band'),
                                           False))
        self.addParameter(ParameterSelection(self.RTYPE,
                                             self.tr('Output raster type'),
                                             self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Merged')))

    def name(self):
        return 'merge'

    def displayName(self):
        return self.tr('Merge')

    def group(self):
        return self.tr('Raster miscellaneous')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        if self.getParameterValue(self.SEPARATE):
            arguments.append('-separate')
        if self.getParameterValue(self.PCT):
            arguments.append('-pct')
        opts = self.getParameterValue(self.OPTIONS)
        if opts:
            arguments.append('-co')
            arguments.append(opts)

        arguments.append('-o')
        out = self.getOutputValue(self.OUTPUT)
        arguments.append(out)
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        arguments.extend(self.getParameterValue(self.INPUT).split(';'))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal_merge.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal_merge.py', GdalUtils.escapeAndJoin(arguments)]

        return commands
