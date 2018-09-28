# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteDuplicateGeometries.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeatureRequest,
                       QgsProcessingException,
                       QgsFeatureSink,
                       QgsSpatialIndex,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DeleteDuplicateGeometries(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Cleaned')))

    def name(self):
        return 'deleteduplicategeometries'

    def displayName(self):
        return self.tr('Delete duplicate geometries')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]))
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        geoms = dict()
        index = QgsSpatialIndex()
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            geoms[f.id()] = f.geometry()
            #index.insertFeature
            feedback.setProgress(int(current * total))

        cleaned = dict(geoms)

        for i, g in list(geoms.items()):
            if feedback.isCanceled():
                break

            for j in list(cleaned.keys()):
                if i == j or i not in cleaned:
                    continue
                if g.isGeosEqual(cleaned[j]):
                    del cleaned[j]

        total = 100.0 / len(cleaned) if cleaned else 1
        request = QgsFeatureRequest().setFilterFids(list(cleaned.keys()))
        for current, f in enumerate(source.getFeatures(request)):
            if feedback.isCanceled():
                break

            sink.addFeature(f, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
