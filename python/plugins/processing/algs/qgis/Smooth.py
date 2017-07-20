# -*- coding: utf-8 -*-

"""
***************************************************************************
    Smooth.py
    ---------
    Date                 : November 2015
    Copyright            : (C) 2015 by Nyall Dawson
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
__date__ = 'November 2015'
__copyright__ = '(C) 2015, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterNumber)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class Smooth(QgisFeatureBasedAlgorithm):

    ITERATIONS = 'ITERATIONS'
    MAX_ANGLE = 'MAX_ANGLE'
    OFFSET = 'OFFSET'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.ITERATIONS,
                                                       self.tr('Iterations'),
                                                       defaultValue=1, minValue=1, maxValue=10))
        self.addParameter(QgsProcessingParameterNumber(self.OFFSET,
                                                       self.tr('Offset'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=0.25, minValue=0.0, maxValue=0.5))
        self.addParameter(QgsProcessingParameterNumber(self.MAX_ANGLE,
                                                       self.tr('Maximum node angle to smooth'), QgsProcessingParameterNumber.Double,
                                                       defaultValue=180.0, minValue=0.0, maxValue=180.0))

    def name(self):
        return 'smoothgeometry'

    def displayName(self):
        return self.tr('Smooth geometry')

    def outputName(self):
        return self.tr('Smoothed')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.iterations = self.parameterAsInt(parameters, self.ITERATIONS, context)
        self.offset = self.parameterAsDouble(parameters, self.OFFSET, context)
        self.max_angle = self.parameterAsDouble(parameters, self.MAX_ANGLE, context)
        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            output_geometry = feature.geometry().smooth(self.iterations, self.offset, -1, self.max_angle)
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error smoothing geometry'))

            feature.setGeometry(output_geometry)
        return feature
