# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteHoles.py
    ---------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Etienne Trimaille
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Etienne Trimaille'
__date__ = 'April 2015'
__copyright__ = '(C) 2015, Etienne Trimaille'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication,
                       QgsFeatureSink,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingParameterDefinition)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DeleteHoles(QgisAlgorithm):

    INPUT = 'INPUT'
    MIN_AREA = 'MIN_AREA'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('remove,delete,drop,holes,rings,fill').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterNumber(self.MIN_AREA,
                                                       self.tr('Remove holes with area less than'), QgsProcessingParameterNumber.Double,
                                                       0, True, 0.0, 10000000.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Cleaned'), QgsProcessingParameterDefinition.TypeVectorPolygon))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Cleaned'), QgsProcessingParameterDefinition.TypeVectorPolygon))

        self.source = None
        self.min_area = None
        self.sink = None
        self.dest_id = None

    def name(self):
        return 'deleteholes'

    def displayName(self):
        return self.tr('Delete holes')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        self.min_area = self.parameterAsDouble(parameters, self.MIN_AREA, context)
        if self.min_area == 0.0:
            self.min_area = -1.0

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         self.source.fields(), self.source.wkbType(), self.source.sourceCrs())
        return True

    def processAlgorithm(self, context, feedback):
        features = self.source.getFeatures()
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if f.hasGeometry():
                f.setGeometry(f.geometry().removeInteriorRings(self.min_area))
            self.sink.addFeature(f, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
