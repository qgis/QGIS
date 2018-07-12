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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsApplication,
                       QgsField,
                       QgsFeatureSink,
                       QgsRaster,
                       QgsProcessing,
                       QgsProcessingParameterRasterLayer,
                       QgsFields,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RasterSampling(QgisAlgorithm):

    INPUT = 'INPUT'
    RASTERCOPY = 'RASTERCOPY'
    OUTPUT = 'OUTPUT'

    def name(self):
        return 'rastersampling'

    def displayName(self):
        return self.tr('Sample Raster Values')

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

        sampled_rasters = self.parameterAsRasterLayer(
            parameters,
            self.RASTERCOPY,
            context
        )

        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        source_fields = source.fields()
        raster_fields = QgsFields()

        # append field to vector as rasterName_bandCount
        for b in range(sampled_rasters.bandCount()):
            raster_fields.append(QgsField(
                'rvalue_' + str('{}'.format(b + 1)), QVariant.Double
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

        for n, i in enumerate(source.getFeatures()):

            if i.geometry().isMultipart():
                raise QgsProcessingException(self.tr('''Impossible to sample data
                of a Multipart layer. Please use the Multipart to single part
                algoithm to transform the layer.'''))

            attrs = i.attributes()

            if sampled_rasters.bandCount() > 1:

                for b in range(sampled_rasters.bandCount()):
                    attrs.append(
                        sampled_rasters.dataProvider().identify(i.geometry().asPoint(),
                                                                QgsRaster.IdentifyFormatValue).results()[b + 1]
                    )

            attrs.append(
                sampled_rasters.dataProvider().identify(i.geometry().asPoint(),
                                                        QgsRaster.IdentifyFormatValue).results()[1]
            )

            i.setAttributes(attrs)

            sink.addFeature(i, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(n * total))

        return {self.OUTPUT: dest_id}
