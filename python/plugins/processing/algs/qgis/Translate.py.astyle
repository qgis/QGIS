# -*- coding: utf-8 -*-

"""
***************************************************************************
    Translate.py
    --------------
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

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterNumber)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class Translate(QgisFeatureBasedAlgorithm):

    DELTA_X = 'DELTA_X'
    DELTA_Y = 'DELTA_Y'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.delta_x = 0
        self.delta_y = 0

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.DELTA_X,
                                                       self.tr('Offset distance (x-axis)'), QgsProcessingParameterNumber.Double, defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.DELTA_Y,
                                                       self.tr('Offset distance (y-axis)'), QgsProcessingParameterNumber.Double, defaultValue=0.0))

    def name(self):
        return 'translategeometry'

    def displayName(self):
        return self.tr('Translate geometry')

    def outputName(self):
        return self.tr('Translated')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.delta_x = self.parameterAsDouble(parameters, self.DELTA_X, context)
        self.delta_y = self.parameterAsDouble(parameters, self.DELTA_Y, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = input_geometry
            output_geometry.translate(self.delta_x, self.delta_y)
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error translating geometry'))

            feature.setGeometry(output_geometry)
        return feature
