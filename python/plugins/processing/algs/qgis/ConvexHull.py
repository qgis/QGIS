# -*- coding: utf-8 -*-

"""
***************************************************************************
    ConvexHull.py
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

from PyQt4.QtCore import QVariant

from qgis.core import QGis, QgsField, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class ConvexHull(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    METHOD = 'METHOD'
    METHODS = ['Create single minimum convex hull',
               'Create convex hulls based on field']

    def defineCharacteristics(self):
        self.name = 'Convex hull'
        self.group = 'Vector geometry tools'
        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Field (optional, only used if creating convex hulls by classes)'),
            self.INPUT, optional=True))
        self.addParameter(ParameterSelection(self.METHOD,
            self.tr('Method'), self.METHODS))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Convex hull')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        useField = self.getParameterValue(self.METHOD) == 1
        fieldName = self.getParameterValue(self.FIELD)

        f = QgsField('value', QVariant.String, '', 255)
        if useField:
            index = layer.fieldNameIndex(fieldName)
            fType = layer.pendingFields()[index].type()
            if fType == QVariant.Int:
                f.setType(QVariant.Int)
                f.setLength(20)
            elif fType == QVariant.Double:
                f.setType(QVariant.Double)
                f.setLength(20)
                f.setPrecision(6)
            else:
                f.setType(QVariant.String)
                f.setLength(255)

        fields = [QgsField('id', QVariant.Int, '', 20),
                  f,
                  QgsField('area', QVariant.Double, '', 20, 6),
                  QgsField('perim', QVariant.Double, '', 20, 6)
                 ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBPolygon, layer.dataProvider().crs())

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()

        current = 0

        fid = 0
        val = None
        features = vector.features(layer)
        if useField:
            unique = layer.uniqueValues(index)
            total = 100.0 / (len(features) * len(unique))
            for i in unique:
                first = True
                hull = []
                features = vector.features(layer)
                for f in features:
                    idVar = f[fieldName]
                    if unicode(idVar).strip() == unicode(i).strip():
                        if first:
                            val = idVar
                            first = False

                        inGeom = QgsGeometry(f.geometry())
                        points = vector.extractPoints(inGeom)
                        hull.extend(points)
                    current += 1
                    progress.setPercentage(int(current * total))

                if len(hull) >= 3:
                    tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
                    try:
                        outGeom = tmpGeom.convexHull()
                        (area, perim) = vector.simpleMeasure(outGeom)
                        outFeat.setGeometry(outGeom)
                        outFeat.setAttributes([fid, val, area, perim])
                        writer.addFeature(outFeat)
                    except:
                        raise GeoAlgorithmExecutionException(
                            self.tr('Exception while computing convex hull'))
                fid += 1
        else:
            hull = []
            total = 100.0 / float(layer.featureCount())
            features = vector.features(layer)
            for f in features:
                inGeom = QgsGeometry(f.geometry())
                points = vector.extractPoints(inGeom)
                hull.extend(points)
                current += 1
                progress.setPercentage(int(current * total))

            tmpGeom = QgsGeometry(outGeom.fromMultiPoint(hull))
            try:
                outGeom = tmpGeom.convexHull()
                (area, perim) = vector.simpleMeasure(outGeom)
                outFeat.setGeometry(outGeom)
                outFeat.setAttributes([0, 'all', area, perim])
                writer.addFeature(outFeat)
            except:
                raise GeoAlgorithmExecutionException(
                    self.tr('Exception while computing convex hull'))

        del writer
