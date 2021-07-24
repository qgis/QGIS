# -*- coding: utf-8 -*-

"""
***************************************************************************
    AidwInterpolation.py
    ---------------------
    Date                 : October 2021
    Copyright            : (C) 2021 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alessandro Pasotti'
__date__ = 'July 2021'
__copyright__ = '(C) 2021, Alessandro Pasotti'

import os
import math

from qgis.PyQt.QtGui import QIcon

from qgis.core import (Qgis,
                       QgsRectangle,
                       QgsRasterLayer,
                       QgsProcessing,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsRasterFileWriter,
                       QgsProcessingException)

from qgis.analysis import QgsAidwInterpolation

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.qgis.ui.InterpolationWidgets import ParameterPixelSize

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class AidwInterpolation(QgisAlgorithm):
    INPUT = 'INPUT'
    FIELD_NAME = 'FIELD_NAME'
    DISTANCE_COEFFICIENT = 'DISTANCE_COEFFICIENT'
    PIXEL_SIZE = 'PIXEL_SIZE'
    COLUMNS = 'COLUMNS'
    ROWS = 'ROWS'
    EXTENT = 'EXTENT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'interpolation.png'))

    def group(self):
        return self.tr('Interpolation')

    def groupId(self):
        return 'interpolation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'), [QgsProcessing.TypeVectorPoint]))

        self.addParameter(QgsProcessingParameterField(self.FIELD_NAME, self.tr('Interpolation attribute'), -1, self.INPUT, QgsProcessingParameterField.Numeric))

        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE_COEFFICIENT, self.tr('Distance coefficient P'), type=QgsProcessingParameterNumber.Double, defaultValue=2.0, minValue=1.0, maxValue=4.0))

        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Extent')))

        pixel_size_param = ParameterPixelSize(self.PIXEL_SIZE,
                                              self.tr('Output raster size'),
                                              layersData=self.INPUT,
                                              extent=self.EXTENT,
                                              minValue=0.0,
                                              default=0.1)
        self.addParameter(pixel_size_param)

        cols_param = QgsProcessingParameterNumber(self.COLUMNS,
                                                  self.tr('Number of columns'),
                                                  optional=True,
                                                  minValue=0, maxValue=10000000)
        cols_param.setFlags(cols_param.flags() | QgsProcessingParameterDefinition.FlagHidden)
        self.addParameter(cols_param)

        rows_param = QgsProcessingParameterNumber(self.ROWS,
                                                  self.tr('Number of rows'),
                                                  optional=True,
                                                  minValue=0, maxValue=10000000)
        rows_param.setFlags(rows_param.flags() | QgsProcessingParameterDefinition.FlagHidden)
        self.addParameter(rows_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Interpolated')))

    def name(self):
        return 'aidwinterpolation'

    def displayName(self):
        return self.tr('Accelerated IDW interpolation')

    def processAlgorithm(self, parameters, context, feedback):

        interpolationData = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        attribute = self.parameterAsFields(parameters, self.FIELD_NAME, context)
        coefficient = self.parameterAsDouble(parameters, self.DISTANCE_COEFFICIENT, context)
        extent = self.parameterAsExtent(parameters, self.EXTENT, context)
        pixel_size = self.parameterAsDouble(parameters, self.PIXEL_SIZE, context)
        output = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        columns = self.parameterAsInt(parameters, self.COLUMNS, context)
        rows = self.parameterAsInt(parameters, self.ROWS, context)
        if columns == 0:
            columns = max(math.ceil(extent.width() / pixel_size), 1)
        if rows == 0:
            rows = max(math.ceil(extent.height() / pixel_size), 1)

        if interpolationData is None:
            raise QgsProcessingException(
                self.tr('You need to specify one input layer.'))

        if len(attribute) != 1:
            raise QgsProcessingException(
                self.tr('You need to specify one numeric attribute.'))

        attribute = attribute[0]

        if interpolationData.fields().lookupField(attribute) < 0 or not interpolationData.fields().field(attribute).isNumeric():
            raise QgsProcessingException(
                self.tr('You need to specify one numeric attribute.'))

        # Create destination layer
        writer = QgsRasterFileWriter(output)
        if not writer.createOneBandRaster(Qgis.Float64, rows, columns, extent, interpolationData.crs()):
            raise QgsProcessingException(
                self.tr('Could not create destination raster.'))

        # Load destination layer
        interpolated_layer = QgsRasterLayer(output, 'Interpolated', 'gdal')
        if not interpolated_layer.isValid():
            raise QgsProcessingException(
                self.tr('Destination raster is not valid.'))

        interpolator = QgsAidwInterpolation(interpolationData, attribute, interpolated_layer, coefficient)

        try:
            interpolator.process(feedback)
        except QgsProcessingException as ex:
            raise QgsProcessingException(
                self.tr('Error executing IDW interpolation algorithm: {}.'.format(ex.what())))

        return {self.OUTPUT: output}
