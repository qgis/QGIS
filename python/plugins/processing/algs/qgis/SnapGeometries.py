# -*- coding: utf-8 -*-

"""
***************************************************************************
    SnapGeometries.py
    -----------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.analysis import (QgsGeometrySnapper,
                           QgsGeometrySnapperSingleSource,
                           QgsInternalGeometrySnapper)
from qgis.core import (QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterEnum)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class SnapGeometriesToLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    REFERENCE_LAYER = 'REFERENCE_LAYER'
    TOLERANCE = 'TOLERANCE'
    OUTPUT = 'OUTPUT'
    BEHAVIOR = 'BEHAVIOR'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'), [QgsProcessing.TypeVectorPoint, QgsProcessing.TypeVectorLine, QgsProcessing.TypeVectorPolygon]))
        self.addParameter(QgsProcessingParameterFeatureSource(self.REFERENCE_LAYER, self.tr('Reference layer'),
                                                              [QgsProcessing.TypeVectorPoint,
                                                               QgsProcessing.TypeVectorLine,
                                                               QgsProcessing.TypeVectorPolygon]))

        self.addParameter(QgsProcessingParameterDistance(self.TOLERANCE, self.tr('Tolerance'), parentParameterName=self.INPUT,
                                                         minValue=0.00000001, maxValue=9999999999, defaultValue=10.0))

        self.modes = [self.tr('Prefer aligning nodes, insert extra vertices where required'),
                      self.tr('Prefer closest point, insert extra vertices where required'),
                      self.tr('Prefer aligning nodes, don\'t insert new vertices'),
                      self.tr('Prefer closest point, don\'t insert new vertices'),
                      self.tr('Move end points only, prefer aligning nodes'),
                      self.tr('Move end points only, prefer closest point'),
                      self.tr('Snap end points to end points only'),
                      self.tr('Snap to anchor nodes (single layer only)')]
        self.addParameter(QgsProcessingParameterEnum(
            self.BEHAVIOR,
            self.tr('Behavior'),
            options=self.modes, defaultValue=0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Snapped geometry')))

    def name(self):
        return 'snapgeometries'

    def displayName(self):
        return self.tr('Snap geometries to layer')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        reference_source = self.parameterAsSource(parameters, self.REFERENCE_LAYER, context)
        if reference_source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.REFERENCE_LAYER))

        tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)
        mode = self.parameterAsEnum(parameters, self.BEHAVIOR, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        if parameters[self.INPUT] != parameters[self.REFERENCE_LAYER]:
            if mode == 7:
                raise QgsProcessingException(self.tr('This mode applies when the input and reference layer are the same.'))

            snapper = QgsGeometrySnapper(reference_source)
            processed = 0
            for f in features:
                if feedback.isCanceled():
                    break
                if f.hasGeometry():
                    out_feature = f
                    out_feature.setGeometry(snapper.snapGeometry(f.geometry(), tolerance, mode))
                    sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
                else:
                    sink.addFeature(f)
                processed += 1
                feedback.setProgress(processed * total)
        elif mode == 7:
            # input layer == ref layer
            modified_count = QgsGeometrySnapperSingleSource.run(source, sink, tolerance, feedback)
            feedback.pushInfo(self.tr('Snapped {} geometries.').format(modified_count))
        else:
            # snapping internally
            snapper = QgsInternalGeometrySnapper(tolerance, mode)
            processed = 0
            for f in features:
                if feedback.isCanceled():
                    break

                out_feature = f
                out_feature.setGeometry(snapper.snapFeature(f))
                sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
                processed += 1
                feedback.setProgress(processed * total)

        return {self.OUTPUT: dest_id}
