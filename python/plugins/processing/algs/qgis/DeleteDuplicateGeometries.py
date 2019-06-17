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

from qgis.core import (QgsFeatureRequest,
                       QgsProcessingException,
                       QgsFeatureSink,
                       QgsSpatialIndex,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputNumber)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class DeleteDuplicateGeometries(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    RETAINED_COUNT = 'RETAINED_COUNT'
    DUPLICATE_COUNT = 'DUPLICATE_COUNT'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def tags(self):
        return self.tr('drop,remove,same,points,coincident,overlapping,filter').split(',')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Cleaned')))

        self.addOutput(QgsProcessingOutputNumber(self.RETAINED_COUNT, self.tr('Count of retained records')))
        self.addOutput(QgsProcessingOutputNumber(self.DUPLICATE_COUNT, self.tr('Count of discarded duplicate records')))

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
        null_geom_features = set()
        index = QgsSpatialIndex()
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                null_geom_features.add(f.id())
                continue

            geoms[f.id()] = f.geometry()
            index.addFeature(f)

            feedback.setProgress(int(0.10 * current * total)) # takes about 10% of time

        # start by assuming everything is unique, and chop away at this list
        unique_features = dict(geoms)

        current = 0
        removed = 0
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
                    removed += 1

            current += 1
            feedback.setProgress(int(0.80 * current * total) + 10)  # takes about 80% of time

        # now, fetch all the feature attributes for the unique features only
        # be super-smart and don't re-fetch geometries
        distinct_geoms = set(unique_features.keys())
        output_feature_ids = distinct_geoms.union(null_geom_features)
        total = 100.0 / len(output_feature_ids) if output_feature_ids else 1

        request = QgsFeatureRequest().setFilterFids(list(output_feature_ids)).setFlags(QgsFeatureRequest.NoGeometry)
        for current, f in enumerate(source.getFeatures(request)):
            if feedback.isCanceled():
                break

            # use already fetched geometry
            if f.id() not in null_geom_features:
                f.setGeometry(unique_features[f.id()])
            sink.addFeature(f, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(0.10 * current * total) + 90) # takes about 10% of time

        feedback.pushInfo(self.tr('{} duplicate features removed'.format(removed)))
        return {self.OUTPUT: dest_id,
                self.DUPLICATE_COUNT: removed,
                self.RETAINED_COUNT: len(output_feature_ids)}
