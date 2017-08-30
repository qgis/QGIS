# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtentFromRasterLayer.py
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsPointXY,
                       QgsGeometry,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterFeatureSink,
                       QgsFields)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtentFromRasterLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'layer_extent.png'))

    def tags(self):
        return self.tr('extent,envelope,bounds,bounding,boundary,layer').split(',')

    def group(self):
        return self.tr('Raster tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Extent'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'polygonfromrasterextent'

    def displayName(self):
        return self.tr('Polygon from raster extent')

    def processAlgorithm(self, parameters, context, feedback):
        raster = self.parameterAsRasterLayer(parameters, self.INPUT, context)

        fields = QgsFields()
        fields.append(QgsField('MINX', QVariant.Double))
        fields.append(QgsField('MINY', QVariant.Double))
        fields.append(QgsField('MAXX', QVariant.Double))
        fields.append(QgsField('MAXY', QVariant.Double))
        fields.append(QgsField('CNTX', QVariant.Double))
        fields.append(QgsField('CNTY', QVariant.Double))
        fields.append(QgsField('AREA', QVariant.Double))
        fields.append(QgsField('PERIM', QVariant.Double))
        fields.append(QgsField('HEIGHT', QVariant.Double))
        fields.append(QgsField('WIDTH', QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, raster.crs())

        self.layerExtent(raster, sink, feedback)

        return {self.OUTPUT: dest_id}

    def layerExtent(self, raster, sink, feedback):
        rect = raster.extent()
        geometry = QgsGeometry.fromRect(rect)
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
        sink.addFeature(feat, QgsFeatureSink.FastInsert)
