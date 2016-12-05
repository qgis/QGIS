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
from processing.modeler.ModelerAlgorithm import ValueFromInput, ValueFromOutput
import os


__author__ = 'Victor Olaya'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import math
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterMultipleInput, ParameterExtent, ParameterString, ParameterRaster, ParameterNumber
from processing.core.outputs import OutputRaster
from processing.tools import dataobjects
from processing.algs.gdal.GdalUtils import GdalUtils
from qgis.core import QgsRectangle
from qgis.analysis import QgsRasterCalculator, QgsRasterCalculatorEntry
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.algs.qgis.ui.RasterCalculatorWidgets import LayersListWidgetWrapper, ExpressionWidgetWrapper


class RasterCalculator(GeoAlgorithm):

    LAYERS = 'LAYERS'
    EXTENT = 'EXTENT'
    CELLSIZE = 'CELLSIZE'
    EXPRESSION = 'EXPRESSION'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Raster calculator')
        self.group, self.i18n_group = self.trAlgorithm('Raster')

        self.addParameter(ParameterMultipleInput(self.LAYERS,
                                                 self.tr('Input layers'),
                                                 datatype=dataobjects.TYPE_RASTER,
                                                 optional=True,
                                                 metadata={'widget_wrapper': LayersListWidgetWrapper}))

        class ParameterRasterCalculatorExpression(ParameterString):

            def evaluateForModeler(self, value, model):
                for i in list(model.inputs.values()):
                    param = i.param
                    if isinstance(param, ParameterRaster):
                        new = "{}@".format(os.path.basename(param.value))
                        old = "{}@".format(param.name)
                        value = value.replace(old, new)

                    for alg in list(model.algs.values()):
                        for out in alg.algorithm.outputs:
                            if isinstance(out, OutputRaster):
                                if out.value:
                                    new = "{}@".format(os.path.basename(out.value))
                                    old = "{}:{}@".format(alg.name, out.name)
                                    value = value.replace(old, new)
                return value

        self.addParameter(ParameterRasterCalculatorExpression(self.EXPRESSION, self.tr('Expression'),
                                                              multiline=True,
                                                              metadata={'widget_wrapper': ExpressionWidgetWrapper}))
        self.addParameter(ParameterNumber(self.CELLSIZE,
                                          self.tr('Cellsize (use 0 or empty to set it automatically)'),
                                          minValue=0.0, default=0.0, optional=True))
        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Output extent'),
                                          optional=True))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        expression = self.getParameterValue(self.EXPRESSION)
        layersValue = self.getParameterValue(self.LAYERS)
        layersDict = {}
        if layersValue:
            layers = [dataobjects.getObjectFromUri(f) for f in layersValue.split(";")]
            layersDict = {os.path.basename(lyr.source()): lyr for lyr in layers}

        for lyr in dataobjects.getRasterLayers():
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

        output = self.getOutputValue(self.OUTPUT)
        extentValue = self.getParameterValue(self.EXTENT)

        if extentValue:
            extent = extentValue.split(',')
            bbox = QgsRectangle(float(extent[0]), float(extent[2]),
                                float(extent[1]), float(extent[3]))
        else:
            if layersDict:
                bbox = list(layersDict.values())[0].extent()
                for lyr in layersDict.values():
                    bbox.combineExtentWith(lyr.extent())
            else:
                raise GeoAlgorithmExecutionException(self.tr("No layers selected"))

        def _cellsize(layer):
            return (layer.extent().xMaximum() - layer.extent().xMinimum()) / layer.width()
        cellsize = self.getParameterValue(self.CELLSIZE) or min([_cellsize(lyr) for lyr in layersDict.values()])
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
            raise GeoAlgorithmExecutionException(self.tr("Error parsing formula"))

    def processBeforeAddingToModeler(self, algorithm, model):
        values = []
        expression = algorithm.params[self.EXPRESSION]
        for i in list(model.inputs.values()):
            param = i.param
            if isinstance(param, ParameterRaster) and "{}@".format(param.name) in expression:
                values.append(ValueFromInput(param.name))

        if algorithm.name:
            dependent = model.getDependentAlgorithms(algorithm.name)
        else:
            dependent = []
        for alg in list(model.algs.values()):
            if alg.name not in dependent:
                for out in alg.algorithm.outputs:
                    if (isinstance(out, OutputRaster)
                            and "{}:{}@".format(alg.name, out.name) in expression):
                        values.append(ValueFromOutput(alg.name, out.name))

        algorithm.params[self.LAYERS] = values
