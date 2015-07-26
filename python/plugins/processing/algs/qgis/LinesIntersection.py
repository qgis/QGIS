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

from qgis.core import QGis, QgsFeatureRequest, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class LinesIntersection(GeoAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'
    FIELD_A = 'FIELD_A'
    FIELD_B = 'FIELD_B'

    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Line intersections')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')

        self.addParameter(ParameterVector(self.INPUT_A,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterVector(self.INPUT_B,
            self.tr('Intersect layer'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterTableField(
            self.FIELD_A,
            self.tr('Input unique ID field'),
            self.INPUT_A,
            optional=True))
        self.addParameter(ParameterTableField(
            self.FIELD_B,
            self.tr('Intersect unique ID field'),
            self.INPUT_B,
            optional=True))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersections')))

    def processAlgorithm(self, progress):
        layerA = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_A))
        layerB = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_B))
        fieldA = self.getParameterValue(self.FIELD_A)
        fieldB = self.getParameterValue(self.FIELD_B)

        idxA = layerA.fieldNameIndex(fieldA)
        idxB = layerB.fieldNameIndex(fieldB)

        fieldList = [layerA.pendingFields()[idxA],
                     layerB.pendingFields()[idxB]]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                QGis.WKBPoint, layerA.dataProvider().crs())

        spatialIndex = vector.spatialindex(layerB)

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        tmpGeom = QgsGeometry()

        features = vector.features(layerA)

        current = 0
        total = 100.0 / float(len(features))
        hasIntersections = False

        for inFeatA in features:
            inGeom = inFeatA.geometry()
            hasIntersections = False
            lines = spatialIndex.intersects(inGeom.boundingBox())

            if len(lines) > 0:
                hasIntersections = True

            if hasIntersections:
                for i in lines:
                    request = QgsFeatureRequest().setFilterFid(i)
                    inFeatB = layerB.getFeatures(request).next()
                    tmpGeom = QgsGeometry(inFeatB.geometry())

                    points = []
                    attrsA = inFeatA.attributes()
                    attrsB = inFeatB.attributes()

                    if inGeom.intersects(tmpGeom):
                        tempGeom = inGeom.intersection(tmpGeom)
                        if tempGeom.type() == QGis.Point:
                            if tempGeom.isMultipart():
                                points = tempGeom.asMultiPoint()
                            else:
                                points.append(tempGeom.asPoint())

                            for j in points:
                                outFeat.setGeometry(tempGeom.fromPoint(j))
                                outFeat.setAttributes([attrsA[idxA],
                                        attrsB[idxB]])
                                writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer
