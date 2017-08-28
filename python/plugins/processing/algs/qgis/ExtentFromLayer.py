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

from qgis.core import (QgsField,
                       QgsFeatureSink,
                       QgsPointXY,
                       QgsGeometry,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterBoolean,
                       QgsFields)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtentFromLayer(QgisAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    BY_FEATURE = 'BY_FEATURE'

    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'layer_extent.png'))

    def tags(self):
        return self.tr('extent,envelope,bounds,bounding,boundary,layer').split(',')

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_LAYER, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBoolean(self.BY_FEATURE,
                                                        self.tr('Calculate extent for each feature separately'), False))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Extent'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'polygonfromlayerextent'

    def displayName(self):
        return self.tr('Polygon from layer extent')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT_LAYER, context)
        byFeature = self.parameterAsBool(parameters, self.BY_FEATURE, context)

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
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())

        if byFeature:
            self.featureExtent(source, context, sink, feedback)
        else:
            self.layerExtent(source, sink, feedback)

        return {self.OUTPUT: dest_id}

    def layerExtent(self, source, sink, feedback):
        rect = source.sourceExtent()
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

    def featureExtent(self, source, context, sink, feedback):
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        feat = QgsFeature()
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

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
            rect = [QgsPointXY(minx, miny), QgsPointXY(minx, maxy), QgsPointXY(maxx,
                                                                               maxy), QgsPointXY(maxx, miny), QgsPointXY(minx, miny)]

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

            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
