# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerCalculator.py
    ---------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Victor Olaya
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

import os

__author__ = 'Victor Olaya'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import math
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingUtils,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingParameterString)
from qgis.analysis import QgsRasterCalculator, QgsRasterCalculatorEntry


class RasterCalculator(QgisAlgorithm):

    LAYERS = 'LAYERS'
    EXTENT = 'EXTENT'
    CELLSIZE = 'CELLSIZE'
    EXPRESSION = 'EXPRESSION'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Raster analysis')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        layer_param = QgsProcessingParameterMultipleLayers(self.LAYERS,
                                                           self.tr('Input layers'),
                                                           layerType=QgsProcessing.TypeRaster,
                                                           optional=True)
        layer_param.setMetadata({'widget_wrapper': 'processing.algs.qgis.ui.RasterCalculatorWidgets.LayersListWidgetWrapper'})
        self.addParameter(layer_param)

        class ParameterRasterCalculatorExpression(QgsProcessingParameterString):

            def __init__(self, name='', description='', multiLine=False):
                super().__init__(name, description, multiLine=multiLine)
                self.setMetadata({
                    'widget_wrapper': 'processing.algs.qgis.ui.RasterCalculatorWidgets.ExpressionWidgetWrapper'
                })

            def type(self):
                return 'raster_calc_expression'

            def clone(self):
                return ParameterRasterCalculatorExpression(self.name(), self.description(), self.multiLine())

            def evaluateForModeler(self, value, model):
                for i in list(model.inputs.values()):
                    param = i.param
                    if isinstance(param, QgsProcessingParameterRasterLayer):
                        new = "{}@".format(os.path.basename(param.value))
                        old = "{}@".format(param.name())
                        value = value.replace(old, new)

                    for alg in list(model.algs.values()):
                        for out in alg.algorithm.outputs:
                            if isinstance(out, QgsProcessingOutputRasterLayer):
                                if out.value:
                                    new = "{}@".format(os.path.basename(out.value))
                                    old = "{}:{}@".format(alg.modeler_name, out.name)
                                    value = value.replace(old, new)
                return value

        self.addParameter(ParameterRasterCalculatorExpression(self.EXPRESSION, self.tr('Expression'),
                                                              multiLine=True))
        self.addParameter(QgsProcessingParameterNumber(self.CELLSIZE,
                                                       self.tr('Cell size (use 0 or empty to set it automatically)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, defaultValue=0.0, optional=True))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Output extent'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Output')))

    def name(self):
        return 'rastercalculator'

    def displayName(self):
        return self.tr('Raster calculator')

    def processAlgorithm(self, parameters, context, feedback):
        expression = self.parameterAsString(parameters, self.EXPRESSION, context)
        layers = self.parameterAsLayerList(parameters, self.LAYERS, context)
        layersDict = {}
        if layers:
            layersDict = {os.path.basename(lyr.source().split(".")[0]): lyr for lyr in layers}

        for lyr in QgsProcessingUtils.compatibleRasterLayers(context.project()):
            name = lyr.name()
            if (name + "@") in expression:
                layersDict[name] = lyr

        entries = []
        for name, lyr in layersDict.items():
            for n in range(lyr.bandCount()):
                entry = QgsRasterCalculatorEntry()
                entry.ref = '{:s}@{:d}'.format(name, n + 1)
                entry.raster = lyr
                entry.bandNumber = n + 1
                entries.append(entry)

        output = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        bbox = self.parameterAsExtent(parameters, self.EXTENT, context)
        if bbox.isNull():
            bbox = QgsProcessingUtils.combineLayerExtents(layers)

        if bbox.isNull():
            if layersDict:
                bbox = list(layersDict.values())[0].extent()
                for lyr in layersDict.values():
                    bbox.combineExtentWith(lyr.extent())
            else:
                raise QgsProcessingException(self.tr("No layers selected"))

        def _cellsize(layer):
            return (layer.extent().xMaximum() - layer.extent().xMinimum()) / layer.width()
        cellsize = self.parameterAsDouble(parameters, self.CELLSIZE, context) or min([_cellsize(lyr) for lyr in layersDict.values()])
        width = math.floor((bbox.xMaximum() - bbox.xMinimum()) / cellsize)
        height = math.floor((bbox.yMaximum() - bbox.yMinimum()) / cellsize)
        driverName = GdalUtils.getFormatShortNameFromFilename(output)
        calc = QgsRasterCalculator(expression,
                                   output,
                                   driverName,
                                   bbox,
                                   width,
                                   height,
                                   entries)

        res = calc.processCalculation()
        if res == QgsRasterCalculator.ParserError:
            raise QgsProcessingException(self.tr("Error parsing formula"))

        return {self.OUTPUT: output}

    def processBeforeAddingToModeler(self, algorithm, model):
        values = []
        expression = algorithm.params[self.EXPRESSION]
        for i in list(model.inputs.values()):
            param = i.param
            if isinstance(param, QgsProcessingParameterRasterLayer) and "{}@".format(param.name) in expression:
                values.append(ValueFromInput(param.name()))

        if algorithm.name:
            dependent = model.getDependentAlgorithms(algorithm.name)
        else:
            dependent = []
        for alg in list(model.algs.values()):
            if alg.modeler_name not in dependent:
                for out in alg.algorithm.outputs:
                    if (isinstance(out, QgsProcessingOutputRasterLayer) and
                            "{}:{}@".format(alg.modeler_name, out.name) in expression):
                        values.append(ValueFromOutput(alg.modeler_name, out.name))

        algorithm.params[self.LAYERS] = values
