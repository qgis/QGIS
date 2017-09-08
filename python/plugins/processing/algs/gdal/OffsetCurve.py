# -*- coding: utf-8 -*-

"""
***************************************************************************
    offsetcurve.py
    ---------------------
    Date                 : Janaury 2015
    Copyright            : (C) 2015 by Giovanni Manghi
    Email                : giovanni dot manghi at naturalgis dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giovanni Manghi'
__date__ = 'January 2015'
__copyright__ = '(C) 2015, Giovanni Manghi'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputVectorLayer)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class OffsetCurve(GdalAlgorithm):

    INPUT = 'INPUT'
    GEOMETRY = 'GEOMETRY'
    DISTANCE = 'DISTANCE'
    OPTIONS = 'OPTIONS'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY,
                                                       self.tr('Geometry column name'),
                                                       defaultValue='geometry'))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Offset distance (left-sided: positive, right-sided: negative)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=10))
        self.addParameter(QgsProcessingParameterString(self.OPTIONS,
                                                       self.tr('Additional creation options'),
                                                       defaultValue='',
                                                       optional=True))

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Offset curve'),
                                                                  QgsProcessing.TypeVectorLine))

    def name(self):
        return 'offsetcurve'

    def displayName(self):
        return self.tr('Offset curve')

    def group(self):
        return self.tr('Vector geoprocessing')

    def commandName(self):
        return 'ogr2ogr'

    def getConsoleCommands(self, parameters, context, feedback):
        ogrLayer, layerName = self.getOgrCompatibleSource(self.INPUT, parameters, context, feedback)
        geometry = self.parameterAsString(parameters, self.GEOMETRY, context)
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        options = self.parameterAsString(parameters, self.OPTIONS, context)
        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        output, outputFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        arguments = []
        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append('-dialect')
        arguments.append('sqlite')
        arguments.append('-sql')

        sql = "SELECT ST_OffsetCurve({}, {}), * FROM '{}'".format(geometry, distance, layerName)
        arguments.append(sql)

        if options:
            arguments.append(options)

        return ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]
