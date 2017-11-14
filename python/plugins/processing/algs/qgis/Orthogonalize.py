# -*- coding: utf-8 -*-

"""
***************************************************************************
    Orthogonalize.py
    ----------------
    Date                 : December 2016
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
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterNumber)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class Orthogonalize(QgisFeatureBasedAlgorithm):

    MAX_ITERATIONS = 'MAX_ITERATIONS'
    DISTANCE_THRESHOLD = 'DISTANCE_THRESHOLD'
    ANGLE_TOLERANCE = 'ANGLE_TOLERANCE'

    def tags(self):
        return self.tr('rectangle,perpendicular,right,angles,square,quadrilateralise').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.max_iterations = None
        self.angle_tolerance = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.ANGLE_TOLERANCE,
                                                       self.tr('Maximum angle tolerance (degrees)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, maxValue=45.0, defaultValue=15.0))

        max_iterations = QgsProcessingParameterNumber(self.MAX_ITERATIONS,
                                                      self.tr('Maximum algorithm iterations'),
                                                      type=QgsProcessingParameterNumber.Integer,
                                                      minValue=1, maxValue=10000, defaultValue=1000)
        max_iterations.setFlags(max_iterations.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(max_iterations)

    def name(self):
        return 'orthogonalize'

    def displayName(self):
        return self.tr('Orthogonalize')

    def outputName(self):
        return self.tr('Orthogonalized')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorPolygon, QgsProcessing.TypeVectorLine]

    def prepareAlgorithm(self, parameters, context, feedback):
        self.max_iterations = self.parameterAsInt(parameters, self.MAX_ITERATIONS, context)
        self.angle_tolerance = self.parameterAsDouble(parameters, self.ANGLE_TOLERANCE, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = input_geometry.orthogonalize(1.0e-8, self.max_iterations, self.angle_tolerance)
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error orthogonalizing geometry'))

            feature.setGeometry(output_geometry)
        return feature
