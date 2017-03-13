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

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsFields,
                       QgsField,
                       QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureRequest)
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
        self.tags = self.tr('create,lines,polygons,convert')
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterBoolean(self.FIELDS,
                                           self.tr('Keep table structure of line layer'), False))
        self.addParameter(ParameterBoolean(self.GEOMETRY,
                                           self.tr('Create geometry columns'), True))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Polygons from lines'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, feedback):
        vlayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputFromName(self.OUTPUT)
        if self.getParameterValue(self.FIELDS):
            fields = vlayer.fields()
        else:
            fields = QgsFields()
        if self.getParameterValue(self.GEOMETRY):
            fieldsCount = fields.count()
            fields.append(QgsField('area', QVariant.Double, 'double', 16, 2))
            fields.append(QgsField('perimeter', QVariant.Double,
                                   'double', 16, 2))
        allLinesList = []
        features = vector.features(vlayer, QgsFeatureRequest().setSubsetOfAttributes([]))
        feedback.pushInfo(self.tr('Processing lines...'))
        total = 40.0 / len(features)
        for current, inFeat in enumerate(features):
            if inFeat.geometry():
                allLinesList.append(inFeat.geometry())
            feedback.setProgress(int(current * total))

        feedback.setProgress(40)

        feedback.pushInfo(self.tr('Noding lines...'))
        allLines = QgsGeometry.unaryUnion(allLinesList)

        feedback.setProgress(45)
        feedback.pushInfo(self.tr('Polygonizing...'))
        polygons = QgsGeometry.polygonize([allLines])
        if polygons.isEmpty():
            raise GeoAlgorithmExecutionException(self.tr('No polygons were created!'))
        feedback.setProgress(50)

        feedback.pushInfo('Saving polygons...')
        writer = output.getVectorWriter(fields, QgsWkbTypes.Polygon, vlayer.crs())
        total = 50.0 / polygons.geometry().numGeometries()
        for i in range(polygons.geometry().numGeometries()):
            outFeat = QgsFeature()
            geom = QgsGeometry(polygons.geometry().geometryN(i).clone())
            outFeat.setGeometry(geom)
            if self.getParameterValue(self.GEOMETRY):
                outFeat.setAttributes([None] * fieldsCount + [geom.geometry().area(),
                                                              geom.geometry().perimeter()])
            writer.addFeature(outFeat)
            feedback.setProgress(50 + int(current * total))
        del writer
