# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterSampling.py
    -----------------------
    Date                 : July 2018
    Copyright            : (C) 2018 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matteo Ghetta'
__date__ = 'July 2018'
__copyright__ = '(C) 2018, Matteo Ghetta'


import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (NULL,
                       QgsApplication,
                       QgsField,
                       QgsFeatureSink,
                       QgsRaster,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterString,
                       QgsProcessingParameterDefinition,
                       QgsCoordinateTransform,
                       QgsFields,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RasterSampling(QgisAlgorithm):

    INPUT = 'INPUT'
    RASTERCOPY = 'RASTERCOPY'
    COLUMN_PREFIX = 'COLUMN_PREFIX'
    OUTPUT = 'OUTPUT'

    def name(self):
        return 'rastersampling'

    def displayName(self):
        return self.tr('Sample raster values')

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr('Input Point Layer'),
                [QgsProcessing.TypeVectorPoint]
            )
        )

        self.addParameter(
            QgsProcessingParameterRasterLayer(
                self.RASTERCOPY,
                self.tr('Raster Layer to sample'),
            )
        )

        columnPrefix = QgsProcessingParameterString(
            self.COLUMN_PREFIX,
            self.tr('Output column prefix'), 'rvalue'
        )
        columnPrefix.setFlags(columnPrefix.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(columnPrefix)

        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT,
                self.tr('Sampled Points')
            )
        )

    def processAlgorithm(self, parameters, context, feedback):

        source = self.parameterAsSource(
            parameters,
            self.INPUT,
            context
        )

        sampled_raster = self.parameterAsRasterLayer(
            parameters,
            self.RASTERCOPY,
            context
        )

        columnPrefix = self.parameterAsString(
            parameters,
            self.COLUMN_PREFIX,
            context
        )

        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        source_fields = source.fields()
        raster_fields = QgsFields()

        # append field to vector as columnPrefix_bandCount
        for b in range(sampled_raster.bandCount()):
            raster_fields.append(QgsField(
                columnPrefix + str('_{}'.format(b + 1)), QVariant.Double
            )
            )

        # combine all the vector fields
        out_fields = QgsProcessingUtils.combineFields(source_fields, raster_fields)

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            out_fields,
            source.wkbType(),
            source.sourceCrs()
        )

        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        features = source.getFeatures()

        # create the coordinates transformation context
        ct = QgsCoordinateTransform(source.sourceCrs(), sampled_raster.crs(), context.transformContext())

        for n, i in enumerate(source.getFeatures()):

            attrs = i.attributes()

            if i.geometry().isMultipart() and i.geometry().constGet().partCount() > 1:
                sink.addFeature(i, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(n * total))
                feedback.reportError(self.tr('Impossible to sample data of multipart feature {}.').format(i.id()))
                continue

            # get the feature geometry as point
            point = QgsPointXY()
            if i.geometry().isMultipart():
                point = i.geometry().asMultiPoint()[0]
            else:
                point = i.geometry().asPoint()

            # reproject to raster crs
            try:
                point = ct.transform(point)
            except QgsCsException:
                for b in range(sampled_raster.bandCount()):
                    attrs.append(None)
                i.setAttributes(attrs)
                sink.addFeature(i, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(n * total))
                feedback.reportError(self.tr('Could not reproject feature {} to raster CRS').format(i.id()))
                continue

            for b in range(sampled_raster.bandCount()):
                value, ok = sampled_raster.dataProvider().sample(point, b + 1)
                if ok:
                    attrs.append(value)
                else:
                    attrs.append(NULL)

            i.setAttributes(attrs)

            sink.addFeature(i, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(n * total))

        return {self.OUTPUT: dest_id}
