# -*- coding: utf-8 -*-

"""
***************************************************************************
    Heatmap.py
    ---------------------
    Date                 : November 2016
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from collections import OrderedDict

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsFeatureRequest,
                       QgsRasterFileWriter,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterField,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterDestination)

from qgis.analysis import QgsKernelDensityEstimation

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Heatmap(QgisAlgorithm):

    INPUT = 'INPUT'
    RADIUS = 'RADIUS'
    RADIUS_FIELD = 'RADIUS_FIELD'
    WEIGHT_FIELD = 'WEIGHT_FIELD'
    PIXEL_SIZE = 'PIXEL_SIZE'
    KERNEL = 'KERNEL'
    DECAY = 'DECAY'
    OUTPUT_VALUE = 'OUTPUT_VALUE'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/heatmap.svg")

    def tags(self):
        return self.tr('heatmap,kde,hotspot').split(',')

    def group(self):
        return self.tr('Interpolation')

    def groupId(self):
        return 'interpolation'

    def name(self):
        return 'heatmapkerneldensityestimation'

    def displayName(self):
        return self.tr('Heatmap (Kernel Density Estimation)')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.KERNELS = OrderedDict([(self.tr('Quartic'), QgsKernelDensityEstimation.KernelQuartic),
                                    (self.tr('Triangular'), QgsKernelDensityEstimation.KernelTriangular),
                                    (self.tr('Uniform'), QgsKernelDensityEstimation.KernelUniform),
                                    (self.tr('Triweight'), QgsKernelDensityEstimation.KernelTriweight),
                                    (self.tr('Epanechnikov'), QgsKernelDensityEstimation.KernelEpanechnikov)])

        self.OUTPUT_VALUES = OrderedDict([(self.tr('Raw'), QgsKernelDensityEstimation.OutputRaw),
                                          (self.tr('Scaled'), QgsKernelDensityEstimation.OutputScaled)])

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Point layer'),
                                                              [QgsProcessing.TypeVectorPoint]))

        self.addParameter(QgsProcessingParameterDistance(self.RADIUS,
                                                         self.tr('Radius'),
                                                         100.0, self.INPUT, False, 0.0, 9999999999.99))

        radius_field_param = QgsProcessingParameterField(self.RADIUS_FIELD,
                                                         self.tr('Radius from field'),
                                                         None,
                                                         self.INPUT,
                                                         QgsProcessingParameterField.Numeric,
                                                         optional=True
                                                         )
        radius_field_param.setFlags(radius_field_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(radius_field_param)

        class ParameterHeatmapPixelSize(QgsProcessingParameterNumber):

            def __init__(self, name='', description='', parent_layer=None, radius_param=None, radius_field_param=None, minValue=None, maxValue=None,
                         default=None, optional=False):
                QgsProcessingParameterNumber.__init__(self, name, description, QgsProcessingParameterNumber.Double, default, optional, minValue, maxValue)
                self.parent_layer = parent_layer
                self.radius_param = radius_param
                self.radius_field_param = radius_field_param

            def clone(self):
                copy = ParameterHeatmapPixelSize(self.name(), self.description(), self.parent_layer, self.radius_param, self.radius_field_param, self.minimum(), self.maximum(), self.defaultValue((), self.flags() & QgsProcessingParameterDefinition.FlagOptional))
                return copy

        pixel_size_param = ParameterHeatmapPixelSize(self.PIXEL_SIZE,
                                                     self.tr('Output raster size'),
                                                     parent_layer=self.INPUT,
                                                     radius_param=self.RADIUS,
                                                     radius_field_param=self.RADIUS_FIELD,
                                                     minValue=0.0,
                                                     maxValue=9999999999,
                                                     default=0.1)
        pixel_size_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.qgis.ui.HeatmapWidgets.HeatmapPixelSizeWidgetWrapper'}})
        self.addParameter(pixel_size_param)

        weight_field_param = QgsProcessingParameterField(self.WEIGHT_FIELD,
                                                         self.tr('Weight from field'),
                                                         None,
                                                         self.INPUT,
                                                         QgsProcessingParameterField.Numeric,
                                                         optional=True
                                                         )
        weight_field_param.setFlags(weight_field_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(weight_field_param)

        keys = list(self.KERNELS.keys())
        kernel_shape_param = QgsProcessingParameterEnum(self.KERNEL,
                                                        self.tr('Kernel shape'),
                                                        keys,
                                                        allowMultiple=False,
                                                        defaultValue=0)
        kernel_shape_param.setFlags(kernel_shape_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(kernel_shape_param)

        decay_ratio = QgsProcessingParameterNumber(self.DECAY,
                                                   self.tr('Decay ratio (Triangular kernels only)'),
                                                   QgsProcessingParameterNumber.Double,
                                                   0.0, True, -100.0, 100.0)
        decay_ratio.setFlags(decay_ratio.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(decay_ratio)

        keys = list(self.OUTPUT_VALUES.keys())
        output_scaling = QgsProcessingParameterEnum(self.OUTPUT_VALUE,
                                                    self.tr('Output value scaling'),
                                                    keys,
                                                    allowMultiple=False,
                                                    defaultValue=0)
        output_scaling.setFlags(output_scaling.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(output_scaling)

        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Heatmap')))

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        radius = self.parameterAsDouble(parameters, self.RADIUS, context)
        kernel_shape = self.parameterAsEnum(parameters, self.KERNEL, context)
        pixel_size = self.parameterAsDouble(parameters, self.PIXEL_SIZE, context)
        decay = self.parameterAsDouble(parameters, self.DECAY, context)
        output_values = self.parameterAsEnum(parameters, self.OUTPUT_VALUE, context)
        outputFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        output_format = QgsRasterFileWriter.driverForExtension(os.path.splitext(outputFile)[1])
        weight_field = self.parameterAsString(parameters, self.WEIGHT_FIELD, context)
        radius_field = self.parameterAsString(parameters, self.RADIUS_FIELD, context)

        attrs = []

        kde_params = QgsKernelDensityEstimation.Parameters()
        kde_params.source = source
        kde_params.radius = radius
        kde_params.pixelSize = pixel_size
        # radius field
        if radius_field:
            kde_params.radiusField = radius_field
            attrs.append(source.fields().lookupField(radius_field))
        # weight field
        if weight_field:
            kde_params.weightField = weight_field
            attrs.append(source.fields().lookupField(weight_field))

        kde_params.shape = kernel_shape
        kde_params.decayRatio = decay
        kde_params.outputValues = output_values

        kde = QgsKernelDensityEstimation(kde_params, outputFile, output_format)

        if kde.prepare() != QgsKernelDensityEstimation.Success:
            raise QgsProcessingException(
                self.tr('Could not create destination layer'))

        request = QgsFeatureRequest()
        request.setSubsetOfAttributes(attrs)
        features = source.getFeatures(request)
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if kde.addFeature(f) != QgsKernelDensityEstimation.Success:
                feedback.reportError(self.tr('Error adding feature with ID {} to heatmap').format(f.id()))

            feedback.setProgress(int(current * total))

        if kde.finalise() != QgsKernelDensityEstimation.Success:
            raise QgsProcessingException(
                self.tr('Could not save destination layer'))

        return {self.OUTPUT: outputFile}
