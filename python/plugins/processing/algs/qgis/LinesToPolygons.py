# -*- coding: utf-8 -*-

"""
***************************************************************************
    LinesToPolygons.py
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

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeature, QgsGeometry, QgsWkbTypes, QgsProcessingUtils

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesToPolygons(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'to_lines.png'))

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'linestopolygons'

    def displayName(self):
        return self.tr('Lines to polygons')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'),
                                          [dataobjects.TYPE_VECTOR_LINE]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Polygons from lines'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(self.getParameterValue(self.INPUT))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), QgsWkbTypes.Polygon, layer.crs()
        )

        outFeat = QgsFeature()
        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for current, f in enumerate(features):
            outGeomList = []
            if f.geometry().isMultipart():
                outGeomList = f.geometry().asMultiPolyline()
            else:
                outGeomList.append(f.geometry().asPolyline())

            polyGeom = self.removeBadLines(outGeomList)
            if len(polyGeom) != 0:
                outFeat.setGeometry(QgsGeometry.fromPolygon(polyGeom))
                attrs = f.attributes()
                outFeat.setAttributes(attrs)
                writer.addFeature(outFeat)

            feedback.setProgress(int(current * total))

        del writer

    def removeBadLines(self, lines):
        geom = []
        if len(lines) == 1:
            if len(lines[0]) > 2:
                geom = lines
            else:
                geom = []
        else:
            geom = [elem for elem in lines if len(elem) > 2]
        return geom
