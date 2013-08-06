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

from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import QVariant
from qgis.core import QgsFeature, QgsField, QgsFields, QgsGeometry, QGis
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class Polygonize(GeoAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELDS = "FIELDS"
    GEOMETRY = "GEOMETRY"

    def processAlgorithm(self, progress):
        try:
            from shapely.ops import polygonize
            from shapely.geometry import Point,MultiLineString
        except ImportError:
            raise GeoAlgorithmExecutionException('Polygonize algorithm requires shapely module!')
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputFromName(self.OUTPUT)
        vprovider = vlayer.dataProvider()
        if self.getParameterValue(self.FIELDS):
            fields = vprovider.fields()
        else:
            fields = QgsFields()
        if self.getParameterValue(self.GEOMETRY):
            fieldsCount = fields.count()
            fields.append(QgsField("area",QVariant.Double,"double",16,2))
            fields.append(QgsField("perimeter",QVariant.Double,"double",16,2))
        allLinesList = []
        features = QGisLayers.features(vlayer)
        current = 0
        total = 40.0 / float(len(features))
        for inFeat in features:
            inGeom = inFeat.geometry()
            if inGeom.isMultipart():
                allLinesList.extend(inGeom.asMultiPolyline())
            else:
                allLinesList.append(inGeom.asPolyline())
            current += 1
            progress.setPercentage(int(current * total))
        progress.setPercentage(40)
        allLines = MultiLineString(allLinesList)
        allLines = allLines.union(Point(0,0))
        progress.setPercentage(45)
        polygons = list(polygonize([allLines]))
        progress.setPercentage(50)
        writer = output.getVectorWriter(fields, QGis.WKBPolygon, vlayer.crs() )
        outFeat = QgsFeature()
        current = 0
        total = 50.0 / float(len(polygons))
        for polygon in polygons:
            outFeat.setGeometry(QgsGeometry.fromWkt( polygon.wkt ))
            if self.getParameterValue(self.GEOMETRY):
                outFeat.setAttributes([None]*fieldsCount + [ polygon.area, polygon.length])
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(50+int(current * total))
        del writer

    def defineCharacteristics(self):
        self.name = "Polygonize"
        self.group = "Vector geometry tools"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterBoolean(self.FIELDS, "Keep table structure of line layer", False))
        self.addParameter(ParameterBoolean(self.GEOMETRY, "Create geometry columns", True))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))
