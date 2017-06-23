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

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

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

    def name(self):
        return 'deleteholes'

    def displayName(self):
        return self.tr('Delete holes')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        min_area = self.parameterAsDouble(parameters, self.MIN_AREA, context)
        if min_area == 0.0:
            min_area = -1.0

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if f.hasGeometry():
                f.setGeometry(f.geometry().removeInteriorRings(min_area))
            sink.addFeature(f)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
