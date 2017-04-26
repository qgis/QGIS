# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtentFromLayer.py
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
from qgis.PyQt.QtCore import QVariant

from qgis.core import QgsField, QgsPoint, QgsGeometry, QgsFeature, QgsWkbTypes, QgsProcessingUtils

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtentFromLayer(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    BY_FEATURE = 'BY_FEATURE'

    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'layer_extent.png'))

    def tags(self):
        return self.tr('extent,envelope,bounds,bounding,boundary,layer').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def name(self):
        return 'polygonfromlayerextent'

    def displayName(self):
        return self.tr('Polygon from layer extent')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterBoolean(self.BY_FEATURE,
                                           self.tr('Calculate extent for each feature separately'), False))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Extent'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT_LAYER))
        byFeature = self.getParameterValue(self.BY_FEATURE)

        fields = [
            QgsField('MINX', QVariant.Double),
            QgsField('MINY', QVariant.Double),
            QgsField('MAXX', QVariant.Double),
            QgsField('MAXY', QVariant.Double),
            QgsField('CNTX', QVariant.Double),
            QgsField('CNTY', QVariant.Double),
            QgsField('AREA', QVariant.Double),
            QgsField('PERIM', QVariant.Double),
            QgsField('HEIGHT', QVariant.Double),
            QgsField('WIDTH', QVariant.Double),
        ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     QgsWkbTypes.Polygon, layer.crs())

        if byFeature:
            self.featureExtent(layer, context, writer, feedback)
        else:
            self.layerExtent(layer, writer, feedback)

        del writer

    def layerExtent(self, layer, writer, feedback):
        rect = layer.extent()
        minx = rect.xMinimum()
        miny = rect.yMinimum()
        maxx = rect.xMaximum()
        maxy = rect.yMaximum()
        height = rect.height()
        width = rect.width()
        cntx = minx + width / 2.0
        cnty = miny + height / 2.0
        area = width * height
        perim = 2 * width + 2 * height

        rect = [QgsPoint(minx, miny), QgsPoint(minx, maxy), QgsPoint(maxx,
                                                                     maxy), QgsPoint(maxx, miny), QgsPoint(minx, miny)]
        geometry = QgsGeometry().fromPolygon([rect])
        feat = QgsFeature()
        feat.setGeometry(geometry)
        attrs = [
            minx,
            miny,
            maxx,
            maxy,
            cntx,
            cnty,
            area,
            perim,
            height,
            width,
        ]
        feat.setAttributes(attrs)
        writer.addFeature(feat)

    def featureExtent(self, layer, context, writer, feedback):
        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        feat = QgsFeature()
        for current, f in enumerate(features):
            rect = f.geometry().boundingBox()
            minx = rect.xMinimum()
            miny = rect.yMinimum()
            maxx = rect.xMaximum()
            maxy = rect.yMaximum()
            height = rect.height()
            width = rect.width()
            cntx = minx + width / 2.0
            cnty = miny + height / 2.0
            area = width * height
            perim = 2 * width + 2 * height
            rect = [QgsPoint(minx, miny), QgsPoint(minx, maxy), QgsPoint(maxx,
                                                                         maxy), QgsPoint(maxx, miny), QgsPoint(minx, miny)]

            geometry = QgsGeometry().fromPolygon([rect])
            feat.setGeometry(geometry)
            attrs = [
                minx,
                miny,
                maxx,
                maxy,
                cntx,
                cnty,
                area,
                perim,
                height,
                width,
            ]
            feat.setAttributes(attrs)

            writer.addFeature(feat)
            feedback.setProgress(int(current * total))
