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

import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteLog import SextanteLog

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString
from sextante.outputs.OutputVector import OutputVector

from sextante.algs.ftools import FToolsUtils as utils

class SumLines(GeoAlgorithm):

    LINES = "LINES"
    POLYGONS = "POLYGONS"
    LEN_FIELD = "LEN_FIELD"
    COUNT_FIELD = "COUNT_FIELD"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/sum_lines.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Sum line lengths"
        self.group = "Vector analysis tools"

        self.addParameter(ParameterVector(self.LINES, "Lines", ParameterVector.VECTOR_TYPE_LINE))
        self.addParameter(ParameterVector(self.POLYGONS, "Polygons", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterString(self.LEN_FIELD, "Lines length field name", "LENGTH"))
        self.addParameter(ParameterString(self.COUNT_FIELD, "Lines count field name", "COUNT"))

        self.addOutput(OutputVector(self.OUTPUT, "Result"))

    def processAlgorithm(self, progress):
        lineLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LINES))
        polyLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.POLYGONS))
        lengthFieldName = self.getParameterValue(self.LEN_FIELD)
        countFieldName = self.getParameterValue(self.COUNT_FIELD)

        polyProvider = polyLayer.dataProvider()

        idxLength, fieldList = utils.findOrCreateField(polyLayer, polyLayer.pendingFields(), lengthFieldName)
        idxCount, fieldList = utils.findOrCreateField(polyLayer, fieldList, countFieldName)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList.toList(),
                     polyProvider.geometryType(), polyProvider.crs())

        spatialIndex = utils.createSpatialIndex(lineLayer)

        ftLine = QgsFeature()
        ftPoly = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        current = 0
        features = QGisLayers.features(polyLayer)
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
