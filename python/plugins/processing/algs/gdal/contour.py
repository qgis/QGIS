# -*- coding: utf-8 -*-

"""
***************************************************************************
    contour.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class contour(GdalAlgorithm):

    INPUT = 'INPUT'
    BAND = 'BAND'
    INTERVAL = 'INTERVAL'
    FIELD_NAME = 'FIELD_NAME'
    CREATE_3D = 'CREATE_3D'
    IGNORE_NODATA = 'IGNORE_NODATA'
    NODATA = 'NODATA'
    OFFSET = 'OFFSET'
    EXTRA = 'EXTRA'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(
            self.BAND, self.tr('Band number'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(
            self.INTERVAL, self.tr('Interval between contour lines'),
            type=QgsProcessingParameterNumber.Double,
            minValue=0.0, maxValue=99999999.999999, defaultValue=10.0))
        self.addParameter(QgsProcessingParameterString(
            self.FIELD_NAME, self.tr('Attribute name (if not set, no elevation attribute is attached)'),
            defaultValue='ELEV', optional=True))

        create_3d_param = QgsProcessingParameterBoolean(self.CREATE_3D,
                                                        self.tr('Produce 3D vector'),
                                                        defaultValue=False)
        create_3d_param.setFlags(create_3d_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(create_3d_param)

        ignore_nodata_param = QgsProcessingParameterBoolean(self.IGNORE_NODATA,
                                                            self.tr('Treat all raster values as valid'),
                                                            defaultValue=False)
        ignore_nodata_param.setFlags(ignore_nodata_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(ignore_nodata_param)

        nodata_param = QgsProcessingParameterNumber(
            self.NODATA, self.tr('Input pixel value to treat as "nodata"'),
            type=QgsProcessingParameterNumber.Double,
            minValue=-99999999.999999, maxValue=99999999.999999, defaultValue=0.0, optional=True)
        nodata_param.setFlags(nodata_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(nodata_param)

        offset_param = QgsProcessingParameterNumber(
            self.OFFSET, self.tr('Offset from zero relative to which to interpret intervals'),
            type=QgsProcessingParameterNumber.Double,
            minValue=0.0, maxValue=99999999.999999, defaultValue=0.0, optional=True)
        nodata_param.setFlags(offset_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(offset_param)

        extra_options_param = QgsProcessingParameterString(self.EXTRA,
                                                           self.tr('Additional creation parameters'),
                                                           defaultValue='',
                                                           optional=True)
        extra_options_param.setFlags(extra_options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        extra_options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        self.addParameter(extra_options_param)

        self.addParameter(QgsProcessingParameterVectorDestination(
            self.OUTPUT, self.tr('Contours'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'contour'

    def displayName(self):
        return self.tr('Contour')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'contour.png'))

    def group(self):
        return self.tr('Raster extraction')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        band = str(self.parameterAsInt(parameters, self.BAND, context))
        interval = str(self.parameterAsDouble(parameters, self.INTERVAL, context))
        fieldName = self.parameterAsString(parameters, self.FIELD_NAME, context)
        nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        offset = self.parameterAsDouble(parameters, self.OFFSET, context)
        extra = self.parameterAsString(parameters, self.EXTRA, context)

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        output, format = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        arguments = []
        if fieldName:
            arguments.append('-a')
            arguments.append(fieldName)

        arguments.append('-i')
        arguments.append(interval)

        if self.parameterAsBool(parameters, self.CREATE_3D, context):
            arguments.append('-3d')

        if self.parameterAsBool(parameters, self.IGNORE_NODATA, context):
            arguments.append('-inodata')

        if nodata:
            arguments.append('-snodata {}'.format(nodata))

        if offset:
            arguments.append('-off {}'.format(offset))

        if format:
            arguments.append('-f {}'.format(format))

        if extra:
            arguments.append(extra)

        arguments.append(inLayer.source())
        arguments.append(output)

        return ['gdal_contour', GdalUtils.escapeAndJoin(arguments)]
