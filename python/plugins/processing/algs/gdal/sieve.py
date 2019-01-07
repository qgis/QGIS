# -*- coding: utf-8 -*-

"""
***************************************************************************
    sieve.py
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

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class sieve(GdalAlgorithm):

    INPUT = 'INPUT'
    THRESHOLD = 'THRESHOLD'
    EIGHT_CONNECTEDNESS = 'EIGHT_CONNECTEDNESS'
    NO_MASK = 'NO_MASK'
    MASK_LAYER = 'MASK_LAYER'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterNumber(self.THRESHOLD,
                                                       self.tr('Threshold'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=10))
        self.addParameter(QgsProcessingParameterBoolean(self.EIGHT_CONNECTEDNESS,
                                                        self.tr('Use 8-connectedness'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_MASK,
                                                        self.tr('Do not use the default validity mask for the input band'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterRasterLayer(self.MASK_LAYER,
                                                            self.tr('Validity mask'),
                                                            optional=True))

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Sieved')))

    def name(self):
        return 'sieve'

    def displayName(self):
        return self.tr('Sieve')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'sieve.png'))

    def commandName(self):
        return 'gdal_sieve'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = []
        arguments.append('-st')
        arguments.append(str(self.parameterAsInt(parameters, self.THRESHOLD, context)))

        if self.parameterAsBool(parameters, self.EIGHT_CONNECTEDNESS, context):
            arguments.append('-8')
        else:
            arguments.append('-4')

        if self.parameterAsBool(parameters, self.NO_MASK, context):
            arguments.append('-nomask')

        mask = self.parameterAsRasterLayer(parameters, self.MASK_LAYER, context)
        if mask:
            arguments.append('-mask {}'.format(mask.source()))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, out)
        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if raster is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        arguments.append(raster.source())
        arguments.append(out)

        commands = [self.commandName() + '.py', GdalUtils.escapeAndJoin(arguments)]
        if isWindows():
            commands.insert(0, 'python3')

        return commands
