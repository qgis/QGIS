# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExtractNodes.py
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
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsField,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExtractNodes(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'extract_nodes.png'))

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Nodes'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'extractnodes'

    def displayName(self):
        return self.tr('Extract nodes')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        fields = source.fields()
        fields.append(QgsField('node_index', QVariant.Int))
        fields.append(QgsField('distance', QVariant.Double))
        fields.append(QgsField('angle', QVariant.Double))

        out_wkb = QgsWkbTypes.Point
        if QgsWkbTypes.hasM(source.wkbType()):
            out_wkb = QgsWkbTypes.addM(out_wkb)
        if QgsWkbTypes.hasZ(source.wkbType()):
            out_wkb = QgsWkbTypes.addZ(out_wkb)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, out_wkb, source.sourceCrs())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            input_geometry = f.geometry()
            if not input_geometry:
                sink.addFeature(f, QgsFeatureSink.FastInsert)
            else:
                i = 0
                for part in input_geometry.geometry().coordinateSequence():
                    for ring in part:
                        if feedback.isCanceled():
                            break

                        for point in ring:
                            distance = input_geometry.distanceToVertex(i)
                            angle = math.degrees(input_geometry.angleAtVertex(i))
                            attrs = f.attributes()
                            attrs.append(i)
                            attrs.append(distance)
                            attrs.append(angle)
                            output_feature = QgsFeature()
                            output_feature.setAttributes(attrs)
                            output_feature.setGeometry(QgsGeometry(point.clone()))
                            sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
                            i += 1

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
