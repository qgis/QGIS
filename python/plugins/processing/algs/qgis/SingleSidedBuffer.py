# -*- coding: utf-8 -*-

"""
***************************************************************************
    SingleSidedBuffer.py
    --------------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingException)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class SingleSidedBuffer(QgisFeatureBasedAlgorithm):
    DISTANCE = 'DISTANCE'
    SIDE = 'SIDE'
    SEGMENTS = 'SEGMENTS'
    JOIN_STYLE = 'JOIN_STYLE'
    MITER_LIMIT = 'MITER_LIMIT'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.distance = None
        self.segments = None
        self.join_style = None
        self.side = None
        self.miter_limit = None
        self.sides = [self.tr('Left'),
                      'Right']
        self.join_styles = [self.tr('Round'),
                            'Miter',
                            'Bevel']

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Distance'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=10.0))
        self.addParameter(QgsProcessingParameterEnum(
            self.SIDE,
            self.tr('Side'),
            options=self.sides))

        self.addParameter(QgsProcessingParameterNumber(self.SEGMENTS,
                                                       self.tr('Segments'), QgsProcessingParameterNumber.Integer,
                                                       minValue=1, defaultValue=8))

        self.addParameter(QgsProcessingParameterEnum(
            self.JOIN_STYLE,
            self.tr('Join style'),
            options=self.join_styles))
        self.addParameter(QgsProcessingParameterNumber(self.MITER_LIMIT,
                                                       self.tr('Miter limit'), QgsProcessingParameterNumber.Double,
                                                       minValue=1, defaultValue=2))

    def name(self):
        return 'singlesidedbuffer'

    def displayName(self):
        return self.tr('Single sided buffer')

    def outputName(self):
        return self.tr('Buffer')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]

    def outputType(self):
        return QgsProcessing.TypeVectorPolygon

    def outputWkbType(self, input_wkb_type):
        return QgsWkbTypes.Polygon

    def prepareAlgorithm(self, parameters, context, feedback):
        self.distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        self.segments = self.parameterAsInt(parameters, self.SEGMENTS, context)
        self.join_style = self.parameterAsEnum(parameters, self.JOIN_STYLE, context) + 1
        if self.parameterAsEnum(parameters, self.SIDE, context) == 0:
            self.side = QgsGeometry.SideLeft
        else:
            self.side = QgsGeometry.SideRight
        self.miter_limit = self.parameterAsDouble(parameters, self.MITER_LIMIT, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = input_geometry.singleSidedBuffer(self.distance, self.segments,
                                                               self.side, self.join_style, self.miter_limit)
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error calculating single sided buffer'))

            feature.setGeometry(output_geometry)

        return feature
