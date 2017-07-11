# -*- coding: utf-8 -*-

"""
***************************************************************************
    SimplifyGeometries.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsMapToPixelSimplifier,
                       QgsMessageLog,
                       QgsFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SimplifyGeometries(QgisAlgorithm):

    INPUT = 'INPUT'
    TOLERANCE = 'TOLERANCE'
    OUTPUT = 'OUTPUT'
    METHOD = 'METHOD'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'simplify.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [dataobjects.TYPE_VECTOR_POLYGON, dataobjects.TYPE_VECTOR_LINE]))
        self.methods = [self.tr('Distance (Douglas-Peucker)'),
                        'Snap to grid',
                        'Area (Visvalingam)']
        self.addParameter(QgsProcessingParameterEnum(
            self.METHOD,
            self.tr('Simplification method'),
            self.methods, defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.TOLERANCE,
                                                       self.tr('Tolerance'), minValue=0.0, maxValue=10000000.0, defaultValue=1.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Simplified')))

    def name(self):
        return 'simplifygeometries'

    def displayName(self):
        return self.tr('Simplify geometries')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)

        pointsBefore = 0
        pointsAfter = 0

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        if method != 0:
            simplifier = QgsMapToPixelSimplifier(QgsMapToPixelSimplifier.SimplifyGeometry, tolerance, method)

        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break
            out_feature = input_feature
            if input_feature.geometry():
                input_geometry = input_feature.geometry()
                pointsBefore += input_geometry.geometry().nCoordinates()

                if method == 0:  # distance
                    output_geometry = input_geometry.simplify(tolerance)
                else:
                    output_geometry = simplifier.simplify(input_geometry)

                pointsAfter += output_geometry.geometry().nCoordinates()
                out_feature.setGeometry(output_geometry)

            sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        QgsMessageLog.logMessage(self.tr('Simplify: Input geometries have been simplified from {0} to {1} points').format(pointsBefore, pointsAfter),
                                 self.tr('Processing'), QgsMessageLog.INFO)

        return {self.OUTPUT: dest_id}
