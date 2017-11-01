# -*- coding: utf-8 -*-

"""
***************************************************************************
    SymmetricalDifference.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsFeatureRequest,
                       NULL,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SymmetricalDifference(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'sym_difference.png'))

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY,
                                                              self.tr('Difference layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Symmetrical difference')))

    def name(self):
        return 'symmetricaldifference'

    def displayName(self):
        return self.tr('Symmetrical difference')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.OVERLAY, context)

        geomType = QgsWkbTypes.multiType(sourceA.wkbType())
        fields = QgsProcessingUtils.combineFields(sourceA.fields(), sourceB.fields())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, geomType, sourceA.sourceCrs())

        featB = QgsFeature()
        outFeat = QgsFeature()

        indexA = QgsSpatialIndex(sourceA, feedback)
        indexB = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())), feedback)

        total = 100.0 / (sourceA.featureCount() * sourceB.featureCount()) if sourceA.featureCount() and sourceB.featureCount() else 1
        count = 0

        for featA in sourceA.getFeatures():
            if feedback.isCanceled():
                break

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

            try:
                outFeat.setGeometry(diffGeom)
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            except:
                QgsMessageLog.logMessage(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'),
                                         self.tr('Processing'), QgsMessageLog.WARNING)
                continue

            count += 1
            feedback.setProgress(int(count * total))

        length = len(sourceA.fields())

        for featA in sourceB.getFeatures(QgsFeatureRequest().setDestinationCrs(sourceA.sourceCrs())):
            if feedback.isCanceled():
                break

            geom = featA.geometry()
            diffGeom = QgsGeometry(geom)
            attrs = featA.attributes()
            attrs = [NULL] * length + attrs
            intersects = indexA.intersects(geom.boundingBox())
            request = QgsFeatureRequest().setFilterFids(intersects).setSubsetOfAttributes([])
            for featB in sourceA.getFeatures(request):
                if feedback.isCanceled():
                    break

                tmpGeom = featB.geometry()
                if diffGeom.intersects(tmpGeom):
                    diffGeom = QgsGeometry(diffGeom.difference(tmpGeom))

            try:
                outFeat.setGeometry(diffGeom)
                outFeat.setAttributes(attrs)
                sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            except:
                QgsMessageLog.logMessage(self.tr('Feature geometry error: One or more output features ignored due to invalid geometry.'),
                                         self.tr('Processing'), QgsMessageLog.WARNING)
                continue

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
