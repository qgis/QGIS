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

from shapely.ops import polygonize
from shapely.ops import unary_union
from shapely.geometry import MultiLineString

from qgis.PyQt.QtCore import QVariant
from qgis.core import QGis, QgsFields, QgsField, QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class Polygonize(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELDS = 'FIELDS'
    GEOMETRY = 'GEOMETRY'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Polygonize')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterBoolean(self.FIELDS,
                                           self.tr('Keep table structure of line layer'), False))
        self.addParameter(ParameterBoolean(self.GEOMETRY,
                                           self.tr('Create geometry columns'), True))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Polygons from lines')))

    def processAlgorithm(self, progress):
        vlayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputFromName(self.OUTPUT)
        vprovider = vlayer.dataProvider()
        if self.getParameterValue(self.FIELDS):
            fields = vprovider.fields()
        else:
            fields = QgsFields()
        if self.getParameterValue(self.GEOMETRY):
            fieldsCount = fields.count()
            fields.append(QgsField('area', QVariant.Double, 'double', 16, 2))
            fields.append(QgsField('perimeter', QVariant.Double,
                                   'double', 16, 2))
        allLinesList = []
        features = vector.features(vlayer)
        progress.setInfo(self.tr('Processing lines...'))
        total = 40.0 / len(features)
        for current, inFeat in enumerate(features):
            inGeom = inFeat.geometry()
            if inGeom.isMultipart():
                allLinesList.extend(inGeom.asMultiPolyline())
            else:
                allLinesList.append(inGeom.asPolyline())
            progress.setPercentage(int(current * total))

        progress.setPercentage(40)
        allLines = MultiLineString(allLinesList)

        progress.setInfo(self.tr('Noding lines...'))
        allLines = unary_union(allLines)

        progress.setPercentage(45)
        progress.setInfo(self.tr('Polygonizing...'))
        polygons = list(polygonize([allLines]))
        if not polygons:
            raise GeoAlgorithmExecutionException(self.tr('No polygons were created!'))
        progress.setPercentage(50)

        progress.setInfo('Saving polygons...')
        writer = output.getVectorWriter(fields, QGis.WKBPolygon, vlayer.crs())
        outFeat = QgsFeature()
        total = 50.0 / len(polygons)
        for current, polygon in enumerate(polygons):
            outFeat.setGeometry(QgsGeometry.fromWkt(polygon.wkt))
            if self.getParameterValue(self.GEOMETRY):
                outFeat.setAttributes([None] * fieldsCount + [polygon.area,
                                                              polygon.length])
            writer.addFeature(outFeat)
            progress.setPercentage(50 + int(current * total))
        del writer
