# -*- coding: utf-8 -*-

"""
***************************************************************************
    TinInterpolation.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsProcessingUtils,
                       QgsProcessing,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsWkbTypes,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException,
                       QgsCoordinateReferenceSystem)
from qgis.analysis import (QgsInterpolator,
                           QgsTinInterpolator,
                           QgsGridFileWriter)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.qgis.ui.InterpolationWidgets import ParameterInterpolationData, ParameterPixelSize

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TinInterpolation(QgisAlgorithm):
    INTERPOLATION_DATA = 'INTERPOLATION_DATA'
    METHOD = 'METHOD'
    PIXEL_SIZE = 'PIXEL_SIZE'
    EXTENT = 'EXTENT'
    OUTPUT = 'OUTPUT'
    TRIANGULATION = 'TRIANGULATION'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'interpolation.png'))

    def group(self):
        return self.tr('Interpolation')

    def groupId(self):
        return 'interpolation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.METHODS = [self.tr('Linear'),
                        self.tr('Clough-Toucher (cubic)')
                        ]

        self.addParameter(ParameterInterpolationData(self.INTERPOLATION_DATA,
                                                     self.tr('Input layer(s)')))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Interpolation method'),
                                                     options=self.METHODS,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Extent'),
                                                       optional=False))
        pixel_size_param = ParameterPixelSize(self.PIXEL_SIZE,
                                              self.tr('Output raster size'),
                                              layersData=self.INTERPOLATION_DATA,
                                              extent=self.EXTENT,
                                              minValue=0.0,
                                              default=0.1)
        self.addParameter(pixel_size_param)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT,
                                                                  self.tr('Interpolated')))

        triangulation_file_param = QgsProcessingParameterFeatureSink(self.TRIANGULATION,
                                                                     self.tr('Triangulation'),
                                                                     type=QgsProcessing.TypeVectorLine,
                                                                     optional=True)
        triangulation_file_param.setCreateByDefault(False)
        self.addParameter(triangulation_file_param)

    def name(self):
        return 'tininterpolation'

    def displayName(self):
        return self.tr('TIN interpolation')

    def processAlgorithm(self, parameters, context, feedback):
        interpolationData = ParameterInterpolationData.parseValue(parameters[self.INTERPOLATION_DATA])
        method = self.parameterAsEnum(parameters, self.METHOD, context)
        bbox = self.parameterAsExtent(parameters, self.EXTENT, context)
        pixel_size = self.parameterAsDouble(parameters, self.PIXEL_SIZE, context)
        output = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        if interpolationData is None:
            raise QgsProcessingException(
                self.tr('You need to specify at least one input layer.'))

        layerData = []
        layers = []
        crs = QgsCoordinateReferenceSystem()
        for row in interpolationData.split(';'):
            v = row.split('::~::')
            data = QgsInterpolator.LayerData()

            # need to keep a reference until interpolation is complete
            layer = QgsProcessingUtils.variantToSource(v[0], context)
            data.source = layer
            layers.append(layer)
            if not crs.isValid():
                crs = layer.sourceCrs()

            data.valueSource = int(v[1])
            data.interpolationAttribute = int(v[2])
            if v[3] == '0':
                data.sourceType = QgsInterpolator.SourcePoints
            elif v[3] == '1':
                data.sourceType = QgsInterpolator.SourceStructureLines
            else:
                data.sourceType = QgsInterpolator.SourceBreakLines
            layerData.append(data)

        if method == 0:
            interpolationMethod = QgsTinInterpolator.Linear
        else:
            interpolationMethod = QgsTinInterpolator.CloughTocher

        (triangulation_sink, triangulation_dest_id) = self.parameterAsSink(parameters, self.TRIANGULATION, context,
                                                                           QgsTinInterpolator.triangulationFields(), QgsWkbTypes.LineString, crs)

        interpolator = QgsTinInterpolator(layerData, interpolationMethod, feedback)
        if triangulation_sink is not None:
            interpolator.setTriangulationSink(triangulation_sink)

        rows = max(round(bbox.height() / pixel_size) + 1, 1)
        columns = max(round(bbox.width() / pixel_size) + 1, 1)

        writer = QgsGridFileWriter(interpolator,
                                   output,
                                   bbox,
                                   columns,
                                   rows)

        writer.writeFile(feedback)
        return {self.OUTPUT: output, self.TRIANGULATION: triangulation_dest_id}
