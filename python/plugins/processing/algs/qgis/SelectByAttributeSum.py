# -*- coding: utf-8 -*-

"""
***************************************************************************
    SelectByAttributeSum.py
    ---------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'April 2015'
__copyright__ = '(C) 2015, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsSpatialIndex, QgsFeatureRequest, QgsGeometry

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class SelectByAttributeSum(GeoAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    VALUE = 'VALUE'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Select by attribute sum'
        self.group = 'Vector selection tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Selection attribute'), self.INPUT, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterNumber(self.VALUE, self.tr('Value')))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output'), True))

    def processAlgorithm(self, progress):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getObjectFromUri(fileName)
        fieldName = self.getParameterValue(self.FIELD)
        value = self.getParameterValue(self.VALUE)

        selected = layer.selectedFeaturesIds()
        if len(selected) == 0:
            GeoAlgorithmExecutionException(
                self.tr('There is no selection in the input layer. '
                        'Select one feature and try again.'))

        ft = layer.selectedFeatures()[0]
        geom = QgsGeometry(ft.geometry())
        attrSum = ft[fieldName]

        idx = QgsSpatialIndex(layer.getFeatures())
        req = QgsFeatureRequest()
        completed = False
        while not completed:
            intersected = idx.intersects(geom.boundingBox())
            if len(intersected) < 0:
                progress.setInfo(self.tr('No adjacent features found.'))
                break

            for i in intersected:
                ft = layer.getFeatures(req.setFilterFid(i)).next()
                tmpGeom = QgsGeometry(ft.geometry())
                if tmpGeom.touches(geom):
                    geom = tmpGeom.combine(geom)
                    selected.append(i)
                    attrSum += ft[fieldName]
                    if attrSum >= value:
                        completed = True
                        break

        layer.setSelectedFeatures(selected)
        self.setOutputValue(self.OUTPUT, fileName)
