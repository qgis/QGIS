# -*- coding: utf-8 -*-

"""
***************************************************************************
    Polygonize.py
    ---------------------
    Date                 : March 2013
    Copyright            : (C) 2013 by Piotr Pociask
    Email                : ppociask at o2 dot pl
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Piotr Pociask'
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Piotr Pociask'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFields,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class Polygonize(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    KEEP_FIELDS = 'KEEP_FIELDS'

    def tags(self):
        return self.tr('create,lines,polygons,convert').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), types=[QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterBoolean(self.KEEP_FIELDS,
                                                        self.tr('Keep table structure of line layer'), defaultValue=False, optional=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Polygons from lines'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'polygonize'

    def displayName(self):
        return self.tr('Polygonize')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if self.parameterAsBool(parameters, self.KEEP_FIELDS, context):
            fields = source.fields()
        else:
            fields = QgsFields()

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())

        allLinesList = []
        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]))
        feedback.pushInfo(self.tr('Processing lines...'))
        total = (40.0 / source.featureCount()) if source.featureCount() else 1
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break

            if inFeat.geometry():
                allLinesList.append(inFeat.geometry())
            feedback.setProgress(int(current * total))

        feedback.setProgress(40)

        feedback.pushInfo(self.tr('Noding lines...'))
        allLines = QgsGeometry.unaryUnion(allLinesList)
        if feedback.isCanceled():
            return {}

        feedback.setProgress(45)
        feedback.pushInfo(self.tr('Polygonizing...'))
        polygons = QgsGeometry.polygonize([allLines])
        if polygons.isEmpty():
            feedback.reportError(self.tr('No polygons were created!'))
        feedback.setProgress(50)

        if not polygons.isEmpty():
            feedback.pushInfo('Saving polygons...')
            total = 50.0 / polygons.constGet().numGeometries()
            for i in range(polygons.constGet().numGeometries()):
                if feedback.isCanceled():
                    break

                outFeat = QgsFeature()
                geom = QgsGeometry(polygons.constGet().geometryN(i).clone())
                outFeat.setGeometry(geom)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                feedback.setProgress(50 + int(current * total))

        return {self.OUTPUT: dest_id}
