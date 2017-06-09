# -*- coding: utf-8 -*-

"""
***************************************************************************
    Clip.py
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
                       QgsFeatureRequest,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingParameterDefinition
                       )

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Clip(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'clip.png'))

    def group(self):
        return self.tr('Vector overlay tools')

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY, self.tr('Clip layer'), [QgsProcessingParameterDefinition.TypeVectorPolygon]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Clipped')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr("Clipped")))

    def name(self):
        return 'clip'

    def displayName(self):
        return self.tr('Clip')

    def processAlgorithm(self, parameters, context, feedback):

        feature_source = self.parameterAsSource(parameters, self.INPUT, context)
        mask_source = self.parameterAsSource(parameters, self.OVERLAY, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               feature_source.fields(), QgsWkbTypes.multiType(feature_source.wkbType()), feature_source.sourceCrs())

        # first build up a list of clip geometries
        clip_geoms = []
        for mask_feature in mask_source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(feature_source.sourceCrs())):
            clip_geoms.append(mask_feature.geometry())

        # are we clipping against a single feature? if so, we can show finer progress reports
        if len(clip_geoms) > 1:
            combined_clip_geom = QgsGeometry.unaryUnion(clip_geoms)
            single_clip_feature = False
        else:
            combined_clip_geom = clip_geoms[0]
            single_clip_feature = True

        # use prepared geometries for faster intersection tests
        engine = QgsGeometry.createGeometryEngine(combined_clip_geom.geometry())
        engine.prepareGeometry()

        tested_feature_ids = set()

        for i, clip_geom in enumerate(clip_geoms):
            if feedback.isCanceled():
                break

            input_features = [f for f in feature_source.getFeatures(QgsFeatureRequest().setFilterRect(clip_geom.boundingBox()))]

            if not input_features:
                continue

            if single_clip_feature:
                total = 100.0 / len(input_features)
            else:
                total = 0

            for current, in_feat in enumerate(input_features):
                if feedback.isCanceled():
                    break

                if not in_feat.geometry():
                    continue

                if in_feat.id() in tested_feature_ids:
                    # don't retest a feature we have already checked
                    continue

                tested_feature_ids.add(in_feat.id())

                if not engine.intersects(in_feat.geometry().geometry()):
                    continue

                if not engine.contains(in_feat.geometry().geometry()):
                    cur_geom = in_feat.geometry()
                    new_geom = combined_clip_geom.intersection(cur_geom)
                    if new_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(new_geom.geometry().wkbType()) == QgsWkbTypes.GeometryCollection:
                        int_com = in_feat.geometry().combine(new_geom)
                        int_sym = in_feat.geometry().symDifference(new_geom)
                        new_geom = int_com.difference(int_sym)
                else:
                    # clip geometry totally contains feature geometry, so no need to perform intersection
                    new_geom = in_feat.geometry()

                try:
                    out_feat = QgsFeature()
                    out_feat.setGeometry(new_geom)
                    out_feat.setAttributes(in_feat.attributes())
                    sink.addFeature(out_feat)
                except:
                    QgsMessageLog.logMessage(self.tr('Feature geometry error: One or more '
                                                     'output features ignored due to '
                                                     'invalid geometry.'), self.tr('Processing'), QgsMessageLog.CRITICAL)
                    continue

                if single_clip_feature:
                    feedback.setProgress(int(current * total))

            if not single_clip_feature:
                # coarse progress report for multiple clip geometries
                feedback.setProgress(100.0 * i / len(clip_geoms))

        return {self.OUTPUT: dest_id}
