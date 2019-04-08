# -*- coding: utf-8 -*-

"""
***************************************************************************
    FindProjection.py
    -----------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsGeometry,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsField,
                       QgsFields,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QgsCoordinateTransformContext,
                       QgsWkbTypes,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterDefinition)
from qgis.PyQt.QtCore import QVariant

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class FindProjection(QgisAlgorithm):

    INPUT = 'INPUT'
    TARGET_AREA = 'TARGET_AREA'
    TARGET_AREA_CRS = 'TARGET_AREA_CRS'
    OUTPUT = 'OUTPUT'

    def tags(self):
        return self.tr('crs,srs,coordinate,reference,system,guess,estimate,finder,determine').split(',')

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        extent_parameter = QgsProcessingParameterExtent(self.TARGET_AREA,
                                                        self.tr('Target area for layer'))
        self.addParameter(extent_parameter)

        # deprecated
        crs_param = QgsProcessingParameterCrs(self.TARGET_AREA_CRS, 'Target area CRS', optional=True)
        crs_param.setFlags(crs_param.flags() | QgsProcessingParameterDefinition.FlagHidden)
        self.addParameter(crs_param)

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('CRS candidates')))

    def name(self):
        return 'findprojection'

    def displayName(self):
        return self.tr('Find projection')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        extent = self.parameterAsExtent(parameters, self.TARGET_AREA, context)
        target_crs = self.parameterAsExtentCrs(parameters, self.TARGET_AREA, context)
        if self.TARGET_AREA_CRS in parameters:
            c = self.parameterAsCrs(parameters, self.TARGET_AREA_CRS, context)
            if c.isValid():
                target_crs = c

        target_geom = QgsGeometry.fromRect(extent)

        fields = QgsFields()
        fields.append(QgsField('auth_id', QVariant.String, '', 20))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.NoGeometry, QgsCoordinateReferenceSystem())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        # make intersection tests nice and fast
        engine = QgsGeometry.createGeometryEngine(target_geom.constGet())
        engine.prepareGeometry()

        layer_bounds = QgsGeometry.fromRect(source.sourceExtent())

        crses_to_check = QgsCoordinateReferenceSystem.validSrsIds()
        total = 100.0 / len(crses_to_check)

        found_results = 0

        transform_context = QgsCoordinateTransformContext()
        for current, srs_id in enumerate(crses_to_check):
            if feedback.isCanceled():
                break

            candidate_crs = QgsCoordinateReferenceSystem.fromSrsId(srs_id)
            if not candidate_crs.isValid():
                continue

            transform_candidate = QgsCoordinateTransform(candidate_crs, target_crs, transform_context)
            transformed_bounds = QgsGeometry(layer_bounds)
            try:
                if not transformed_bounds.transform(transform_candidate) == 0:
                    continue
            except:
                continue

            try:
                if engine.intersects(transformed_bounds.constGet()):
                    feedback.pushInfo(self.tr('Found candidate CRS: {}').format(candidate_crs.authid()))
                    f = QgsFeature(fields)
                    f.setAttributes([candidate_crs.authid()])
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    found_results += 1
            except:
                continue

            feedback.setProgress(int(current * total))

        if found_results == 0:
            feedback.reportError(self.tr('No matching projections found'))

        return {self.OUTPUT: dest_id}
