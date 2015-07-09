# -*- coding: utf-8 -*-

"""
***************************************************************************
    SumLines.py
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

from qgis.core import QgsFeature, QgsGeometry, QgsFeatureRequest, QgsDistanceArea
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SumLines(GeoAlgorithm):

    LINES = 'LINES'
    POLYGONS = 'POLYGONS'
    LEN_FIELD = 'LEN_FIELD'
    COUNT_FIELD = 'COUNT_FIELD'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Sum line lengths'
        self.group = 'Vector analysis tools'

        self.addParameter(ParameterVector(self.LINES,
            self.tr('Lines'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterVector(self.POLYGONS,
            self.tr('Polygons'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addParameter(ParameterString(self.LEN_FIELD,
            self.tr('Lines length field name', 'LENGTH')))
        self.addParameter(ParameterString(self.COUNT_FIELD,
            self.tr('Lines count field name', 'COUNT')))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Line length')))

    def processAlgorithm(self, progress):
        lineLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.LINES))
        polyLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.POLYGONS))
        lengthFieldName = self.getParameterValue(self.LEN_FIELD)
        countFieldName = self.getParameterValue(self.COUNT_FIELD)

        polyProvider = polyLayer.dataProvider()

        (idxLength, fieldList) = vector.findOrCreateField(polyLayer,
                polyLayer.pendingFields(), lengthFieldName)
        (idxCount, fieldList) = vector.findOrCreateField(polyLayer, fieldList,
                countFieldName)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fieldList.toList(), polyProvider.geometryType(), polyProvider.crs())

        spatialIndex = vector.spatialindex(lineLayer)

        ftLine = QgsFeature()
        ftPoly = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        current = 0
        features = vector.features(polyLayer)
        total = 100.0 / float(len(features))
        hasIntersections = False
        for ftPoly in features:
            inGeom = QgsGeometry(ftPoly.geometry())
            attrs = ftPoly.attributes()
            count = 0
            length = 0
            hasIntersections = False
            lines = spatialIndex.intersects(inGeom.boundingBox())
            if len(lines) > 0:
                hasIntersections = True

            if hasIntersections:
                for i in lines:
                    request = QgsFeatureRequest().setFilterFid(i)
                    ftLine = lineLayer.getFeatures(request).next()
                    tmpGeom = QgsGeometry(ftLine.geometry())
                    if inGeom.intersects(tmpGeom):
                        outGeom = inGeom.intersection(tmpGeom)
                        length += distArea.measure(outGeom)
                        count += 1

            outFeat.setGeometry(inGeom)
            if idxLength == len(attrs):
                attrs.append(length)
            else:
                attrs[idxLength] = length
            if idxCount == len(attrs):
                attrs.append(count)
            else:
                attrs[idxCount] = count
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer
