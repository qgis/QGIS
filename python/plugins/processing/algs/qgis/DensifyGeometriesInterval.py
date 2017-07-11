# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensifyGeometriesInterval.py by Anita Graser, Dec 2012
    based on DensifyGeometries.py
    ---------------------
    Date                 : October 2012
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
from builtins import range

__author__ = 'Anita Graser'
__date__ = 'Dec 2012'
__copyright__ = '(C) 2012, Anita Graser'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from math import sqrt

from qgis.core import (QgsWkbTypes,
                       QgsApplication,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingParameterDefinition)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DensifyGeometriesInterval(QgisAlgorithm):

    INPUT = 'INPUT'
    INTERVAL = 'INTERVAL'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorPolygon, QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterNumber(self.INTERVAL,
                                                       self.tr('Interval between vertices to add'), QgsProcessingParameterNumber.Double,
                                                       1, False, 0, 10000000))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Densified')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Densified')))

    def name(self):
        return 'densifygeometriesgivenaninterval'

    def displayName(self):
        return self.tr('Densify geometries given an interval')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        interval = self.parameterAsDouble(parameters, self.INTERVAL, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            feature = f
            if feature.hasGeometry():
                new_geometry = feature.geometry().densifyByDistance(float(interval))
                feature.setGeometry(new_geometry)
            sink.addFeature(feature, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
