# -*- coding: utf-8 -*-

"""
***************************************************************************
    MeanCoords.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsField,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsFields,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingException)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MeanCoords(QgisAlgorithm):
    INPUT = 'INPUT'
    WEIGHT = 'WEIGHT'
    OUTPUT = 'OUTPUT'
    UID = 'UID'
    WEIGHT = 'WEIGHT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'mean.png'))

    def group(self):
        return self.tr('Vector analysis')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.WEIGHT, self.tr('Weight field'),
                                                      parentLayerParameterName=MeanCoords.INPUT,
                                                      type=QgsProcessingParameterField.Numeric,
                                                      optional=True))
        self.addParameter(QgsProcessingParameterField(self.UID,
                                                      self.tr('Unique ID field'),
                                                      parentLayerParameterName=MeanCoords.INPUT,
                                                      optional=True))

        self.addParameter(QgsProcessingParameterFeatureSink(MeanCoords.OUTPUT, self.tr('Mean coordinates'),
                                                            QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'meancoordinates'

    def displayName(self):
        return self.tr('Mean coordinate(s)')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        weight_field = self.parameterAsString(parameters, self.WEIGHT, context)
        unique_field = self.parameterAsString(parameters, self.UID, context)

        attributes = []
        if not weight_field:
            weight_index = -1
        else:
            weight_index = source.fields().lookupField(weight_field)
        if weight_index >= 0:
            attributes.append(weight_index)

        if not unique_field:
            unique_index = -1
        else:
            unique_index = source.fields().lookupField(unique_field)
        if unique_index >= 0:
            attributes.append(unique_index)

        field_list = QgsFields()
        field_list.append(QgsField('MEAN_X', QVariant.Double, '', 24, 15))
        field_list.append(QgsField('MEAN_Y', QVariant.Double, '', 24, 15))
        if unique_index >= 0:
            field_list.append(QgsField('UID', QVariant.String, '', 255))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               field_list, QgsWkbTypes.Point, source.sourceCrs())

        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(attributes))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        means = {}
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            if unique_index == -1:
                clazz = "Single class"
            else:
                clazz = str(feat.attributes()[unique_index]).strip()
            if weight_index == -1:
                weight = 1.00
            else:
                try:
                    weight = float(feat.attributes()[weight_index])
                except:
                    weight = 1.00

            if weight < 0:
                raise QgsProcessingException(
                    self.tr('Negative weight value found. Please fix your data and try again.'))

            if clazz not in means:
                means[clazz] = (0, 0, 0)

            (cx, cy, totalweight) = means[clazz]
            geom = QgsGeometry(feat.geometry())
            geom = vector.extractPoints(geom)
            for i in geom:
                cx += i.x() * weight
                cy += i.y() * weight
                totalweight += weight
            means[clazz] = (cx, cy, totalweight)

        current = 0
        total = 100.0 / len(means) if means else 1
        for (clazz, values) in list(means.items()):
            if feedback.isCanceled():
                break

            outFeat = QgsFeature()
            cx = values[0] / values[2]
            cy = values[1] / values[2]
            meanPoint = QgsPointXY(cx, cy)

            outFeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
            attributes = [cx, cy]
            if unique_index >= 0:
                attributes.append(clazz)
            outFeat.setAttributes(attributes)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            current += 1
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
