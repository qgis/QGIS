# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtendLines.py
    --------------------
    Date                 : October 2016
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingParameterNumber,
                       QgsProcessingException,
                       QgsProcessing)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class ExtendLines(QgisFeatureBasedAlgorithm):

    START_DISTANCE = 'START_DISTANCE'
    END_DISTANCE = 'END_DISTANCE'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.start_distance = None
        self.end_distance = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.START_DISTANCE,
                                                       self.tr('Start distance'), defaultValue=0.0))
        self.addParameter(QgsProcessingParameterNumber(self.END_DISTANCE,
                                                       self.tr('End distance'), defaultValue=0.0))

    def name(self):
        return 'extendlines'

    def displayName(self):
        return self.tr('Extend lines')

    def outputName(self):
        return self.tr('Extended')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]

    def prepareAlgorithm(self, parameters, context, feedback):
        self.start_distance = self.parameterAsDouble(parameters, self.START_DISTANCE, context)
        self.end_distance = self.parameterAsDouble(parameters, self.END_DISTANCE, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = input_geometry.extendLine(self.start_distance, self.end_distance)
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error calculating extended line'))

            feature.setGeometry(output_geometry)

        return feature
