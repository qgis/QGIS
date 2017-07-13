# -*- coding: utf-8 -*-

"""
***************************************************************************
    PolygonsToLines.py
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

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class PolygonsToLines(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'to_lines.png'))

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorPolygon]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Polygons to lines'),
                                                            QgsProcessing.TypeVectorLine))

    def name(self):
        return 'polygonstolines'

    def displayName(self):
        return self.tr('Polygons to lines')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        geomType = QgsWkbTypes.MultiLineString

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geomType, source.sourceCrs())

        outFeat = QgsFeature()

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        count = 0

        for feat in source.getFeatures():
            if feedback.isCanceled():
                break

            if feat.hasGeometry():
                lines = QgsGeometry(feat.geometry().geometry().boundary()).asMultiPolyline()
                outFeat.setGeometry(QgsGeometry.fromMultiPolyline(lines))
                attrs = feat.attributes()
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
