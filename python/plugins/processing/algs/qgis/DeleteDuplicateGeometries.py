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
            index.addFeature(f)

            feedback.setProgress(int(0.10 * current * total)) # takes about 10% of time

        # start by assuming everything is unique, and chop away at this list
        unique_features = dict(geoms)

        current = 0
        for feature_id, geometry in geoms.items():
            if feedback.isCanceled():
                break

            if feature_id not in unique_features:
                # feature was already marked as a duplicate
                continue

            candidates = index.intersects(geometry.boundingBox())
            candidates.remove(feature_id)

            for candidate_id in candidates:
                if candidate_id not in unique_features:
                    # candidate already marked as a duplicate (not sure if this is possible,
                    # since it would mean the current feature would also have to be a duplicate!
                    # but let's be safe!)
                    continue

                if geometry.isGeosEqual(geoms[candidate_id]):
                    # candidate is a duplicate of feature
                    del unique_features[candidate_id]

            current += 1
            feedback.setProgress(int(0.80 * current * total) + 10)  # takes about 80% of time

        total = 100.0 / len(unique_features) if unique_features else 1

        # now, fetch all the feature attributes for the unique features only
        # be super-smart and don't re-fetch geometries
        request = QgsFeatureRequest().setFilterFids(list(unique_features.keys())).setFlags(QgsFeatureRequest.NoGeometry)
        for current, f in enumerate(source.getFeatures(request)):
            if feedback.isCanceled():
                break

            # use already fetched geometry
            f.setGeometry(unique_features[f.id()])
            sink.addFeature(f, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(0.10 * current * total) + 90) # takes about 10% of time

        return {self.OUTPUT: dest_id}
