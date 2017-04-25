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

from qgis.core import (QgsApplication,
                       QgsSpatialIndex,
                       QgsFeatureRequest)

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.tools import dataobjects


class SelectByAttributeSum(GeoAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    VALUE = 'VALUE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector selection tools')

    def name(self):
        return 'selectbyattributesum'

    def displayName(self):
        return self.tr('Select by attribute sum')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Selection attribute'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterNumber(self.VALUE, self.tr('Value')))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Selected (attribute sum)'), True))

    def processAlgorithm(self, context, feedback):
        fileName = self.getParameterValue(self.INPUT)
        layer = dataobjects.getLayerFromString(fileName)
        fieldName = self.getParameterValue(self.FIELD)
        value = self.getParameterValue(self.VALUE)

        selected = layer.selectedFeatureIds()
        if len(selected) == 0:
            GeoAlgorithmExecutionException(
                self.tr('There is no selection in the input layer. '
                        'Select one feature and try again.'))

        ft = layer.selectedFeatures()[0]
        geom = ft.geometry()
        attrSum = ft[fieldName]

        idx = QgsSpatialIndex(layer.getFeatures(QgsFeatureRequest.setSubsetOfAttributes([])))
        req = QgsFeatureRequest()
        completed = False
        while not completed:
            intersected = idx.intersects(geom.boundingBox())
            if len(intersected) < 0:
                feedback.pushInfo(self.tr('No adjacent features found.'))
                break

            req = QgsFeatureRequest().setFilterFids(intersected).setSubsetOfAttributes([fieldName], layer.fields())
            for ft in layer.getFeatures(req):
                tmpGeom = ft.geometry()
                if tmpGeom.touches(geom):
                    geom = tmpGeom.combine(geom)
                    selected.append(ft.id())
                    attrSum += ft[fieldName]
                    if attrSum >= value:
                        completed = True
                        break

        layer.selectByIds(selected)
        self.setOutputValue(self.OUTPUT, fileName)
