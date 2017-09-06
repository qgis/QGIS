# -*- coding: utf-8 -*-

"""
***************************************************************************
    LinesIntersection.py
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

from qgis.core import (QgsFeatureRequest, QgsFeature, QgsGeometry,
                       QgsFeatureSink,
                       QgsWkbTypes,
                       QgsFields,
                       QgsSpatialIndex,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesIntersection(QgisAlgorithm):

    INPUT = 'INPUT'
    INTERSECT = 'INTERSECT'
    INPUT_FIELDS = 'INPUT_FIELDS'
    INTERSECT_FIELDS = 'INTERSECT_FIELDS'

    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'lines_intersection.png'))

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'), [QgsProcessing.TypeVectorLine]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.INTERSECT,
                                                              self.tr('Intersect layer'), [QgsProcessing.TypeVectorLine]))

        self.addParameter(QgsProcessingParameterField(
            self.INPUT_FIELDS,
            self.tr('Input fields to keep (leave empty to keep all fields)'),
            parentLayerParameterName=self.INPUT,
            optional=True, allowMultiple=True))
        self.addParameter(QgsProcessingParameterField(
            self.INTERSECT_FIELDS,
            self.tr('Intersect fields to keep (leave empty to keep all fields)'),
            parentLayerParameterName=self.INTERSECT,
            optional=True, allowMultiple=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Intersections'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'lineintersections'

    def displayName(self):
        return self.tr('Line intersections')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.INTERSECT, context)

        fieldsA = self.parameterAsFields(parameters, self.INPUT_FIELDS, context)
        fieldsB = self.parameterAsFields(parameters, self.INTERSECT_FIELDS, context)

        fieldListA = QgsFields()
        field_indices_a = []
        if len(fieldsA) > 0:
            for f in fieldsA:
                idxA = sourceA.fields().lookupField(f)
                if idxA >= 0:
                    field_indices_a.append(idxA)
                    fieldListA.append(sourceA.fields()[idxA])
        else:
            fieldListA = sourceA.fields()
            field_indices_a = [i for i in range(0, fieldListA.count())]

        fieldListB = QgsFields()
        field_indices_b = []
        if len(fieldsB) > 0:
            for f in fieldsB:
                idxB = sourceB.fields().lookupField(f)
                if idxB >= 0:
                    field_indices_b.append(idxB)
                    fieldListB.append(sourceB.fields()[idxB])
        else:
            fieldListB = sourceB.fields()
            field_indices_b = [i for i in range(0, fieldListB.count())]

        fieldListB = vector.testForUniqueness(fieldListA, fieldListB)
        for b in fieldListB:
            fieldListA.append(b)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fieldListA, QgsWkbTypes.Point, sourceA.sourceCrs())

        spatialIndex = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())), feedback)

        outFeat = QgsFeature()
        features = sourceA.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(field_indices_a))
        total = 100.0 / sourceA.featureCount() if sourceA.featureCount() else 0
        for current, inFeatA in enumerate(features):
            if feedback.isCanceled():
                break

            if not inFeatA.hasGeometry():
                continue

            inGeom = inFeatA.geometry()
            has_intersections = False
            lines = spatialIndex.intersects(inGeom.boundingBox())

            engine = None
            if len(lines) > 0:
                has_intersections = True
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                engine.prepareGeometry()

            if has_intersections:
                request = QgsFeatureRequest().setFilterFids(lines)
                request.setDestinationCrs(sourceA.sourceCrs())
                request.setSubsetOfAttributes(field_indices_b)

                for inFeatB in sourceB.getFeatures(request):
                    if feedback.isCanceled():
                        break

                    tmpGeom = inFeatB.geometry()

                    points = []
                    if engine.intersects(tmpGeom.geometry()):
                        tempGeom = inGeom.intersection(tmpGeom)
                        out_attributes = [inFeatA.attributes()[i] for i in field_indices_a]
                        out_attributes.extend([inFeatB.attributes()[i] for i in field_indices_b])
                        if tempGeom.type() == QgsWkbTypes.PointGeometry:
                            if tempGeom.isMultipart():
                                points = tempGeom.asMultiPoint()
                            else:
                                points.append(tempGeom.asPoint())

                            for j in points:
                                outFeat.setGeometry(tempGeom.fromPoint(j))
                                outFeat.setAttributes(out_attributes)
                                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
