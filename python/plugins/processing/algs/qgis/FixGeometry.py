# -*- coding: utf-8 -*-

"""
***************************************************************************
    FixGeometry.py
    -----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Alexander Bruy
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
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsWkbTypes,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsProcessingFeatureSource,
                       QgsGeometry,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class FixGeometry(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('repair,invalid,geometry').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'), [QgsProcessingParameterDefinition.TypeVectorLine, QgsProcessingParameterDefinition.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Fixed geometries')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr("Fixed geometries")))

        self.source = None
        self.sink = None
        self.dest_id = None

    def name(self):
        return 'fixgeometries'

    def displayName(self):
        return self.tr('Fix geometries')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         self.source.fields(), QgsWkbTypes.multiType(self.source.wkbType()), self.source.sourceCrs())
        return True

    def processAlgorithm(self, context, feedback):
        features = self.source.getFeatures(QgsFeatureRequest(), QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0
        for current, inputFeature in enumerate(features):
            if feedback.isCanceled():
                break

            outputFeature = inputFeature
            if inputFeature.geometry():
                outputGeometry = inputFeature.geometry().makeValid()
                if not outputGeometry:
                    feedback.pushInfo('makeValid failed for feature {}'.format(inputFeature.id()))

                if outputGeometry.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(outputGeometry.geometry().wkbType()) == QgsWkbTypes.GeometryCollection:
                    tmpGeometries = outputGeometry.asGeometryCollection()
                    for g in tmpGeometries:
                        if g.type() == inputFeature.geometry().type():
                            try:
                                g.convertToMultiType()
                                outputFeature.setGeometry(QgsGeometry(g))
                                self.sink.addFeature(outputFeature, QgsFeatureSink.FastInsert)
                            except:
                                pass
                    feedback.setProgress(int(current * total))
                    continue

                outputGeometry.convertToMultiType()
                outputFeature.setGeometry(outputGeometry)

            self.sink.addFeature(outputFeature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
