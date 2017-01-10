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

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsFeatureRequest
from qgis.analysis import QgsKernelDensityEstimation

from processing.core.ProcessingLog import ProcessingLog
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputRaster
from processing.tools import dataobjects, vector, raster
from processing.algs.qgis.ui.HeatmapWidgets import HeatmapPixelSizeWidgetWrapper

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Heatmap(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    RADIUS = 'RADIUS'
    RADIUS_FIELD = 'RADIUS_FIELD'
    WEIGHT_FIELD = 'WEIGHT_FIELD'
    PIXEL_SIZE = 'PIXEL_SIZE'

    KERNELS = ['Quartic',
               'Triangular',
               'Uniform',
               'Triweight',
               'Epanechnikov'
               ]
    KERNEL = 'KERNEL'
    DECAY = 'DECAY'
    OUTPUT_VALUES = ['Raw',
                     'Scaled'
                     ]
    OUTPUT_VALUE = 'OUTPUT_VALUE'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'heatmap.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Heatmap (Kernel Density Estimation)')
        self.group, self.i18n_group = self.trAlgorithm('Interpolation')
        self.tags = self.tr('heatmap,kde,hotspot')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Point layer'), [dataobjects.TYPE_VECTOR_POINT]))
        self.addParameter(ParameterNumber(self.RADIUS,
                                          self.tr('Radius (layer units)'),
                                          0.0, 9999999999, 100.0))

        radius_field_param = ParameterTableField(self.RADIUS_FIELD,
                                                 self.tr('Radius from field'), self.INPUT_LAYER, optional=True, datatype=ParameterTableField.DATA_TYPE_NUMBER)
        radius_field_param.isAdvanced = True
        self.addParameter(radius_field_param)

        class ParameterHeatmapPixelSize(ParameterNumber):

            def __init__(self, name='', description='', parent_layer=None, radius_param=None, radius_field_param=None, minValue=None, maxValue=None,
                         default=None, optional=False, metadata={}):
                ParameterNumber.__init__(self, name, description, minValue, maxValue, default, optional, metadata)
                self.parent_layer = parent_layer
                self.radius_param = radius_param
                self.radius_field_param = radius_field_param

        self.addParameter(ParameterHeatmapPixelSize(self.PIXEL_SIZE,
                                                    self.tr('Output raster size'), parent_layer=self.INPUT_LAYER, radius_param=self.RADIUS,
                                                    radius_field_param=self.RADIUS_FIELD,
                                                    minValue=0.0, maxValue=9999999999, default=0.1,
                                                    metadata={'widget_wrapper': HeatmapPixelSizeWidgetWrapper}))

        weight_field_param = ParameterTableField(self.WEIGHT_FIELD,
                                                 self.tr('Weight from field'), self.INPUT_LAYER, optional=True, datatype=ParameterTableField.DATA_TYPE_NUMBER)
        weight_field_param.isAdvanced = True
        self.addParameter(weight_field_param)
        kernel_shape_param = ParameterSelection(self.KERNEL,
                                                self.tr('Kernel shape'), self.KERNELS)
        kernel_shape_param.isAdvanced = True
        self.addParameter(kernel_shape_param)
        decay_ratio = ParameterNumber(self.DECAY,
                                      self.tr('Decay ratio (Triangular kernels only)'),
                                      -100.0, 100.0, 0.0)
        decay_ratio.isAdvanced = True
        self.addParameter(decay_ratio)
        output_scaling = ParameterSelection(self.OUTPUT_VALUE,
                                            self.tr('Output value scaling'), self.OUTPUT_VALUES)
        output_scaling.isAdvanced = True
        self.addParameter(output_scaling)
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Heatmap')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))

        radius = self.getParameterValue(self.RADIUS)
        kernel_shape = self.getParameterValue(self.KERNEL)
        pixel_size = self.getParameterValue(self.PIXEL_SIZE)
        decay = self.getParameterValue(self.DECAY)
        output_values = self.getParameterValue(self.OUTPUT_VALUE)
        output = self.getOutputValue(self.OUTPUT_LAYER)
        output_format = raster.formatShortNameFromFileName(output)
        weight_field = self.getParameterValue(self.WEIGHT_FIELD)
        radius_field = self.getParameterValue(self.RADIUS_FIELD)

        attrs = []

        kde_params = QgsKernelDensityEstimation.Parameters()
        kde_params.vectorLayer = layer
        kde_params.radius = radius
        kde_params.pixelSize = pixel_size
        # radius field
        if radius_field:
            kde_params.radiusField = radius_field
            attrs.append(layer.fields().lookupField(radius_field))
        # weight field
        if weight_field:
            kde_params.weightField = weight_field
            attrs.append(layer.fields().lookupField(weight_field))

        kde_params.shape = kernel_shape
        kde_params.decayRatio = decay
        kde_params.outputValues = output_values

        kde = QgsKernelDensityEstimation(kde_params, output, output_format)

        if kde.prepare() != QgsKernelDensityEstimation.Success:
            raise GeoAlgorithmExecutionException(
                self.tr('Could not create destination layer'))

        request = QgsFeatureRequest()
        request.setSubsetOfAttributes(attrs)
        features = vector.features(layer, request)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            if kde.addFeature(f) != QgsKernelDensityEstimation.Success:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                                       self.tr('Error adding feature with ID {} to heatmap').format(f.id()))

            progress.setPercentage(int(current * total))

        if kde.finalise() != QgsKernelDensityEstimation.Success:
            raise GeoAlgorithmExecutionException(
                self.tr('Could not save destination layer'))
