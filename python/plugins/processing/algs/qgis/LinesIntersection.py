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
from builtins import next

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (Qgis, QgsFeatureRequest, QgsFeature, QgsGeometry,
                       QgsWkbTypes, QgsFields)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesIntersection(GeoAlgorithm):

    INPUT_A = 'INPUT_A'
    INPUT_B = 'INPUT_B'
    FIELD_A = 'FIELD_A'
    FIELD_B = 'FIELD_B'

    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'lines_intersection.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Line intersections')
        self.group, self.i18n_group = self.trAlgorithm('Vector overlay tools')

        self.addParameter(ParameterVector(self.INPUT_A,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterVector(self.INPUT_B,
                                          self.tr('Intersect layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterTableField(
            self.FIELD_A,
            self.tr('Input field to keep (leave as [not set] to keep all fields)'),
            self.INPUT_A,
            optional=True))
        self.addParameter(ParameterTableField(
            self.FIELD_B,
            self.tr('Intersect field to keep (leave as [not set] to keep all fields)'),
            self.INPUT_B,
            optional=True))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Intersections'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, feedback):
        layerA = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_A))
        layerB = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT_B))
        fieldA = self.getParameterValue(self.FIELD_A)
        fieldB = self.getParameterValue(self.FIELD_B)

        idxA = layerA.fields().lookupField(fieldA)
        idxB = layerB.fields().lookupField(fieldB)

        if idxA != -1:
            fieldListA = QgsFields()
            fieldListA.append(layerA.fields()[idxA])
        else:
            fieldListA = layerA.fields()

        if idxB != -1:
            fieldListB = QgsFields()
            fieldListB.append(layerB.fields()[idxB])
        else:
            fieldListB = layerB.fields()

        fieldListB = vector.testForUniqueness(fieldListA, fieldListB)
        fieldListA.extend(fieldListB)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldListA,
                                                                     QgsWkbTypes.Point, layerA.crs())

        spatialIndex = vector.spatialindex(layerB)

        outFeat = QgsFeature()
        features = vector.features(layerA)
        total = 100.0 / len(features)
        hasIntersections = False

        for current, inFeatA in enumerate(features):
            inGeom = inFeatA.geometry()
            hasIntersections = False
            lines = spatialIndex.intersects(inGeom.boundingBox())

            engine = None
            if len(lines) > 0:
                hasIntersections = True
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(inGeom.geometry())
                engine.prepareGeometry()

            if hasIntersections:
                request = QgsFeatureRequest().setFilterFids(lines)
                for inFeatB in layerB.getFeatures(request):
                    tmpGeom = inFeatB.geometry()

                    points = []
                    attrsA = inFeatA.attributes()
                    if idxA != -1:
                        attrsA = [attrsA[idxA]]
                    attrsB = inFeatB.attributes()
                    if idxB != -1:
                        attrsB = [attrsB[idxB]]

                    if engine.intersects(tmpGeom.geometry()):
                        tempGeom = inGeom.intersection(tmpGeom)
                        if tempGeom.type() == QgsWkbTypes.PointGeometry:
                            if tempGeom.isMultipart():
                                points = tempGeom.asMultiPoint()
                            else:
                                points.append(tempGeom.asPoint())

                            for j in points:
                                outFeat.setGeometry(tempGeom.fromPoint(j))
                                attrsA.extend(attrsB)
                                outFeat.setAttributes(attrsA)
                                writer.addFeature(outFeat)

            feedback.setProgress(int(current * total))

        del writer
