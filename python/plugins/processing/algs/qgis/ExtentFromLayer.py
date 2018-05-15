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

from qgis.core import (QgsApplication,
                       QgsField,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterMapLayer,
                       QgsProcessingParameterFeatureSink,
                       QgsFields)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtentFromLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    BY_FEATURE = 'BY_FEATURE'

    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmExtractLayerExtent.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmExtractLayerExtent.svg")

    def tags(self):
        return self.tr('polygon,from,vector,raster,extent,envelope,bounds,bounding,boundary,layer').split(',')

    def group(self):
        return self.tr('Layer tools')

    def groupId(self):
        return 'layertools'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMapLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Extent'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'polygonfromlayerextent'

    def displayName(self):
        return self.tr('Extract layer extent')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsLayer(parameters, self.INPUT, context)

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
                                               fields, QgsWkbTypes.Polygon, layer.crs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        try:
            # may not be possible
            layer.updateExtents()
        except:
            pass

        rect = layer.extent()
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

        return {self.OUTPUT: dest_id}
