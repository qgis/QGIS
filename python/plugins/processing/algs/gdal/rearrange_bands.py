# -*- coding: utf-8 -*-

"""
***************************************************************************
    translate.py
    ---------------------
    Date                 : August 2018
    Copyright            : (C) 2018 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Mathieu Pellerin'
__date__ = 'August 2018'
__copyright__ = '(C) 2018, Mathieu Pellerin'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import re

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsRasterFileWriter,
                       QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterString,
                       QgsProcessingParameterRasterDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rearrange_bands(GdalAlgorithm):

    INPUT = 'INPUT'
    BANDS = 'BANDS'
    OPTIONS = 'OPTIONS'
    DATA_TYPE = 'DATA_TYPE'
    OUTPUT = 'OUTPUT'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BANDS,
                                                     self.tr('Selected band(s)'),
                                                     None,
                                                     self.INPUT,
                                                     allowMultiple=True))

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        self.addParameter(options_param)

        dataType_param = QgsProcessingParameterEnum(self.DATA_TYPE,
                                                    self.tr('Output data type'),
                                                    self.TYPES,
                                                    allowMultiple=False,
                                                    defaultValue=5)
        dataType_param.setFlags(dataType_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(dataType_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Converted')))

    def name(self):
        return 'rearrange_bands'

    def displayName(self):
        return self.tr('Rearrange bands')

    def group(self):
        return self.tr('Raster conversion')

    def groupId(self):
        return 'rasterconversion'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'translate.png'))

    def shortHelpString(self):
        return self.tr("This algorithm creates a new raster using selected band(s) from a given raster layer.\n\n"
                       "The reordering of bands is possible by dragging individual band names in the multiple selection dialog.")

    def commandName(self):
        return 'gdal_translate'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        out = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        arguments = []

        bands = self.parameterAsFields(parameters, self.BANDS, context)
        for band in bands:
            match = re.search('(?:\A|[^0-9])([0-9]+)(?:\Z|[^0-9]|)', band)
            if match:
                band_nb = match.group(1)
                arguments.append('-b {}'.format(band_nb))

        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        arguments.append('-of')
        arguments.append(QgsRasterFileWriter.driverForExtension(os.path.splitext(out)[1]))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        arguments.append(inLayer.source())
        arguments.append(out)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
