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

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessing,
                       QgsProcessingException,
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
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(self.INTERVAL,
                                                       self.tr('Interval between contour lines'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0,
                                                       defaultValue=10.0))
        self.addParameter(QgsProcessingParameterString(self.FIELD_NAME,
                                                       self.tr('Attribute name (if not set, no elevation attribute is attached)'),
                                                       defaultValue='ELEV',
                                                       optional=True))

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

        nodata_param = QgsProcessingParameterNumber(self.NODATA,
                                                    self.tr('Input pixel value to treat as "nodata"'),
                                                    type=QgsProcessingParameterNumber.Double,
                                                    defaultValue=None,
                                                    optional=True)
        nodata_param.setFlags(nodata_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(nodata_param)

        offset_param = QgsProcessingParameterNumber(self.OFFSET,
                                                    self.tr('Offset from zero relative to which to interpret intervals'),
                                                    type=QgsProcessingParameterNumber.Double,
                                                    defaultValue=0.0,
                                                    optional=True)
        nodata_param.setFlags(offset_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(offset_param)

        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setFlags(options_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(options_param)

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

    def groupId(self):
        return 'rasterextraction'

    def commandName(self):
        return 'gdal_contour'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        if inLayer is None:
            raise QgsProcessingException(self.invalidRasterError(parameters, self.INPUT))

        fieldName = self.parameterAsString(parameters, self.FIELD_NAME, context)
        if self.NODATA in parameters and parameters[self.NODATA] is not None:
            nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        else:
            nodata = None
        offset = self.parameterAsDouble(parameters, self.OFFSET, context)

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        output, outFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        arguments = []
        arguments.append('-b')
        arguments.append(str(self.parameterAsInt(parameters, self.BAND, context)))

        if fieldName:
            arguments.append('-a')
            arguments.append(fieldName)

        arguments.append('-i')
        arguments.append(str(self.parameterAsDouble(parameters, self.INTERVAL, context)))

        if self.parameterAsBool(parameters, self.CREATE_3D, context):
            arguments.append('-3d')

        if self.parameterAsBool(parameters, self.IGNORE_NODATA, context):
            arguments.append('-inodata')

        if nodata is not None:
            arguments.append('-snodata {}'.format(nodata))

        if offset:
            arguments.append('-off {}'.format(offset))

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.append(options)

        if outFormat:
            arguments.append('-f {}'.format(outFormat))

        arguments.append(inLayer.source())
        arguments.append(output)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
