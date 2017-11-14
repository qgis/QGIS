# -*- coding: utf-8 -*-

"""
***************************************************************************
    Difference.py
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
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsFeatureRequest,
                       QgsWkbTypes,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Difference(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'difference.png'))

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY,
                                                              self.tr('Difference layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Difference')))

    def name(self):
        return 'difference'

    def displayName(self):
        return self.tr('Difference')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.OVERLAY, context)

        geomType = QgsWkbTypes.multiType(sourceA.wkbType())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               sourceA.fields(), geomType, sourceA.sourceCrs())

        featB = QgsFeature()
        outFeat = QgsFeature()

        indexB = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())), feedback)

        total = 100.0 / (sourceA.featureCount() * sourceB.featureCount()) if sourceA.featureCount() and sourceB.featureCount() else 1
        count = 0

        for featA in sourceA.getFeatures():
            if feedback.isCanceled():
                break

            if featA.hasGeometry():
                geom = featA.geometry()
                diffGeom = QgsGeometry(geom)
                attrs = featA.attributes()
                intersects = indexB.intersects(geom.boundingBox())
                request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
                request.setDestinationCrs(sourceA.sourceCrs())
                for featB in sourceB.getFeatures(request):
                    if feedback.isCanceled():
                        break
                    tmpGeom = featB.geometry()
                    if diffGeom.intersects(tmpGeom):
                        diffGeom = QgsGeometry(diffGeom.difference(tmpGeom))

                outFeat.setGeometry(diffGeom)
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                sink.addFeature(featA, QgsFeatureSink.FastInsert)

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
