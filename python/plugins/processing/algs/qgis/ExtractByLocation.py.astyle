# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractByLocation.py
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

from qgis.core import (QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsApplication,
                       QgsProcessingUtils)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import vector


class ExtractByLocation(QgisAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    PREDICATE = 'PREDICATE'
    PRECISION = 'PRECISION'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('extract,filter,location,intersects,contains,within').split(',')

    def group(self):
        return self.tr('Vector selection')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.predicates = (
            ('intersects', self.tr('intersects')),
            ('contains', self.tr('contains')),
            ('disjoint', self.tr('disjoint')),
            ('equals', self.tr('equals')),
            ('touches', self.tr('touches')),
            ('overlaps', self.tr('overlaps')),
            ('within', self.tr('within')),
            ('crosses', self.tr('crosses')))

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Layer to select from')))
        self.addParameter(ParameterVector(self.INTERSECT,
                                          self.tr('Additional layer (intersection layer)')))
        self.addParameter(ParameterSelection(self.PREDICATE,
                                             self.tr('Geometric predicate'),
                                             self.predicates,
                                             multiple=True))
        self.addParameter(ParameterNumber(self.PRECISION,
                                          self.tr('Precision'),
                                          0.0, None, 0.0))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Extracted (location)')))

    def name(self):
        return 'extractbylocation'

    def displayName(self):
        return self.tr('Extract by location')

    def processAlgorithm(self, parameters, context, feedback):
        filename = self.getParameterValue(self.INPUT)
        layer = QgsProcessingUtils.mapLayerFromString(filename, context)
        filename = self.getParameterValue(self.INTERSECT)
        selectLayer = QgsProcessingUtils.mapLayerFromString(filename, context)
        predicates = self.getParameterValue(self.PREDICATE)
        precision = self.getParameterValue(self.PRECISION)

        index = QgsProcessingUtils.createSpatialIndex(layer, context)

        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter(layer.fields(), layer.wkbType(), layer.crs(), context)

        if 'disjoint' in predicates:
            disjoinSet = []
            for feat in QgsProcessingUtils.getFeatures(layer, context):
                disjoinSet.append(feat.id())

        selectedSet = []
        features = QgsProcessingUtils.getFeatures(selectLayer, context)
        total = 100.0 / selectLayer.featureCount() if selectLayer.featureCount() else 0
        for current, f in enumerate(features):
            geom = vector.snapToPrecision(f.geometry(), precision)
            bbox = geom.boundingBox()
            bbox.grow(0.51 * precision)
            intersects = index.intersects(bbox)
            request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
            for feat in layer.getFeatures(request):
                tmpGeom = vector.snapToPrecision(feat.geometry(), precision)
                res = False
                for predicate in predicates:
                    if predicate == 'disjoint':
                        if tmpGeom.intersects(geom):
                            try:
                                disjoinSet.remove(feat.id())
                            except:
                                pass  # already removed
                    else:
                        res = getattr(tmpGeom, predicate)(geom)
                        if res:
                            selectedSet.append(feat.id())
                            break

            feedback.setProgress(int(current * total))

        if 'disjoint' in predicates:
            selectedSet = selectedSet + disjoinSet

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / layer.featureCount() if layer.featureCount() else 0
        for current, f in enumerate(features):
            if f.id() in selectedSet:
                writer.addFeature(f, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        del writer
