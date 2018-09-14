# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConcaveHull.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2012 by Piotr Pociask
    Email                : piotr dot pociask at gis-support dot pl
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
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Piotr Pociask'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QCoreApplication
from math import sqrt

from qgis.core import (QgsApplication,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFeatureSink)
import processing
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class ConcaveHull(QgisAlgorithm):

    INPUT = 'INPUT'
    ALPHA = 'ALPHA'
    HOLES = 'HOLES'
    NO_MULTIGEOMETRY = 'NO_MULTIGEOMETRY'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input point layer'), [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterNumber(self.ALPHA,
                                                       self.tr('Threshold (0-1, where 1 is equivalent with Convex Hull)'),
                                                       minValue=0, maxValue=1, defaultValue=0.3, type=QgsProcessingParameterNumber.Double))

        self.addParameter(QgsProcessingParameterBoolean(self.HOLES,
                                                        self.tr('Allow holes'), defaultValue=True))
        self.addParameter(QgsProcessingParameterBoolean(self.NO_MULTIGEOMETRY,
                                                        self.tr('Split multipart geometry into singleparts geometries'), defaultValue=False))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Concave hull'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'concavehull'

    def displayName(self):
        return self.tr('Concave hull (alpha shapes)')

    def shortDescription(self):
        return self.tr('Creates a concave hull using the alpha shapes algorithm.')

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmConcaveHull.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmConcaveHull.svg")

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsSource(parameters, ConcaveHull.INPUT, context)
        if layer is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        alpha = self.parameterAsDouble(parameters, self.ALPHA, context)
        holes = self.parameterAsBool(parameters, self.HOLES, context)
        no_multigeom = self.parameterAsBool(parameters, self.NO_MULTIGEOMETRY, context)

        # Delaunay triangulation from input point layer
        feedback.setProgressText(QCoreApplication.translate('ConcaveHull', 'Creating Delaunay triangles…'))
        delaunay_layer = processing.run("qgis:delaunaytriangulation", {'INPUT': parameters[ConcaveHull.INPUT], 'OUTPUT': 'memory:'}, feedback=feedback, context=context)['OUTPUT']

        # Get max edge length from Delaunay triangles
        feedback.setProgressText(QCoreApplication.translate('ConcaveHull', 'Computing edges max length…'))

        features = delaunay_layer.getFeatures()
        count = delaunay_layer.featureCount()
        if count == 0:
            raise QgsProcessingException(self.tr('No Delaunay triangles created.'))

        counter = 50. / count
        lengths = []
        edges = {}
        for feat in features:
            if feedback.isCanceled():
                break

            line = feat.geometry().asPolygon()[0]
            for i in range(len(line) - 1):
                lengths.append(sqrt(line[i].sqrDist(line[i + 1])))
            edges[feat.id()] = max(lengths[-3:])
            feedback.setProgress(feat.id() * counter)
        max_length = max(lengths)

        # Get features with longest edge longer than alpha*max_length
        feedback.setProgressText(QCoreApplication.translate('ConcaveHull', 'Removing features…'))
        counter = 50. / len(edges)
        i = 0
        ids = []
        for id, max_len in list(edges.items()):
            if feedback.isCanceled():
                break

            if max_len > alpha * max_length:
                ids.append(id)
            feedback.setProgress(50 + i * counter)
            i += 1

        # Remove features
        delaunay_layer.dataProvider().deleteFeatures(ids)

        # Dissolve all Delaunay triangles
        feedback.setProgressText(QCoreApplication.translate('ConcaveHull', 'Dissolving Delaunay triangles…'))
        dissolved_layer = processing.run("native:dissolve", {'INPUT': delaunay_layer, 'OUTPUT': 'memory:'}, feedback=feedback, context=context)['OUTPUT']

        # Save result
        feedback.setProgressText(QCoreApplication.translate('ConcaveHull', 'Saving data…'))
        feat = QgsFeature()
        dissolved_layer.getFeatures().nextFeature(feat)

        # Not needed anymore, free up some resources
        del delaunay_layer
        del dissolved_layer

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               layer.fields(), QgsWkbTypes.Polygon, layer.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        geom = feat.geometry()
        if no_multigeom and geom.isMultipart():
            # Only singlepart geometries are allowed
            geom_list = geom.asGeometryCollection()
            for single_geom in geom_list:
                if feedback.isCanceled():
                    break

                single_feature = QgsFeature()
                if not holes:
                    # Delete holes
                    single_geom = single_geom.removeInteriorRings()
                single_feature.setGeometry(single_geom)
                sink.addFeature(single_feature, QgsFeatureSink.FastInsert)
        else:
            # Multipart geometries are allowed
            if not holes:
                # Delete holes
                geom = geom.removeInteriorRings()
                feat.setGeometry(geom)
            sink.addFeature(feat, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
