# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsLayerFromTable.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class PointsLayerFromTable(GeoAlgorithm):

    INPUT = 'INPUT'
    XFIELD = 'XFIELD'
    YFIELD = 'YFIELD'
    OUTPUT = 'OUTPUT'
    TARGET_CRS = 'TARGET_CRS'

    def processAlgorithm(self, progress):
        source = self.getParameterValue(self.INPUT)
        vlayer = dataobjects.getObjectFromUri(source)
        output = self.getOutputFromName(self.OUTPUT)
        vprovider = vlayer.dataProvider()
        fields = vprovider.fields()
        writer = output.getVectorWriter(fields, QGis.WKBPoint, self.crs)
        xfieldindex = vlayer.fieldNameIndex(
                self.getParameterValue(self.XFIELD))
        yfieldindex = vlayer.fieldNameIndex(
                self.getParameterValue(self.YFIELD))

        crsId = self.getParameterValue(self.TARGET_CRS)
        targetCrs = QgsCoordinateReferenceSystem(crsId)
        self.crs = targetCrs

        outFeat = QgsFeature()
        nElement = 0
        features = vector.features(vlayer)
        nFeat = len(features)
        for feature in features:
            nElement += 1
            progress.setPercentage(nElement * 100 / nFeat)
            attrs = feature.attributes()
            try:
                x = float(attrs[xfieldindex])
                y = float(attrs[yfieldindex])
            except:
                continue
            pt = QgsPoint(x, y)
            outFeat.setGeometry(QgsGeometry.fromPoint(pt))
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)

        del writer

    def defineCharacteristics(self):
        self.name = 'Points layer from table'
        self.group = 'Vector creation tools'
        self.addParameter(ParameterTable(self.INPUT, 'Input layer'))
        self.addParameter(ParameterTableField(self.XFIELD, 'X field',
                          self.INPUT, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterTableField(self.YFIELD, 'Y field',
                          self.INPUT, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterCrs(self.TARGET_CRS, 'Target CRS',
                          'EPSG:4326'))
        self.addOutput(OutputVector(self.OUTPUT, 'Output layer'))
