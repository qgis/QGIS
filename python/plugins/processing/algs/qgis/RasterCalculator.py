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

__author__ = 'Victor Olaya'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import math
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingUtils,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingParameterString,
                       QgsCoordinateTransform,
                       QgsMapLayer)
from qgis.PyQt.QtCore import QObject
from qgis.analysis import QgsRasterCalculator, QgsRasterCalculatorEntry


class RasterCalculator(QgisAlgorithm):

    LAYERS = 'LAYERS'
    EXTENT = 'EXTENT'
    CELLSIZE = 'CELLSIZE'
    EXPRESSION = 'EXPRESSION'
    CRS = 'CRS'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Raster analysis')

    def groupId(self):
        return 'rasteranalysis'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
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

        self.addParameter(ParameterRasterCalculatorExpression(self.EXPRESSION, self.tr('Expression'),
                                                              multiLine=True))
        self.addParameter(QgsProcessingParameterMultipleLayers(self.LAYERS,
                                                               self.tr('Reference layer(s) (used for automated extent, cellsize, and CRS)'),
                                                               layerType=QgsProcessing.TypeRaster,
                                                               optional=True))
        self.addParameter(QgsProcessingParameterNumber(self.CELLSIZE,
                                                       self.tr('Cell size (use 0 or empty to set it automatically)'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       minValue=0.0, defaultValue=0.0, optional=True))
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Output extent'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.CRS, 'Output CRS', optional=True))
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
            layersDict = {lyr.source(): lyr for lyr in layers}

        crs = self.parameterAsCrs(parameters, self.CRS, context)
        if not crs or not crs.isValid():
            if not layers:
                raise QgsProcessingException(self.tr("No reference layer selected nor CRS provided"))
            else:
                crs = list(layersDict.values())[0].crs()

        bbox = self.parameterAsExtent(parameters, self.EXTENT, context)
        if bbox.isNull():
            if not layers:
                raise QgsProcessingException(self.tr("No reference layer selected nor extent box provided"))

        if not bbox.isNull():
            bboxCrs = self.parameterAsExtentCrs(parameters, self.EXTENT, context)
            if bboxCrs != crs:
                transform = QgsCoordinateTransform(bboxCrs, crs, context.project())
                bbox = transform.transformBoundingBox(bbox)

        if bbox.isNull() and layers:
            bbox = QgsProcessingUtils.combineLayerExtents(layers, crs)

        cellsize = self.parameterAsDouble(parameters, self.CELLSIZE, context)
        if cellsize == 0 and not layers:
            raise QgsProcessingException(self.tr("No reference layer selected nor cellsize value provided"))

        def _cellsize(layer):
            ext = layer.extent()
            if layer.crs() != crs:
                transform = QgsCoordinateTransform(layer.crs(), crs, context.project())
                ext = transform.transformBoundingBox(ext)
            return (ext.xMaximum() - ext.xMinimum()) / layer.width()
        if cellsize == 0:
            cellsize = min([_cellsize(lyr) for lyr in layersDict.values()])

        # check for layers available in the model
        layersDictCopy = layersDict.copy() # need a shallow copy because next calls invalidate iterator
        for lyr in layersDictCopy.values():
            expression = self.mappedNameToLayer(lyr, expression, layersDict, context)

        # check for layers available in the project
        for lyr in QgsProcessingUtils.compatibleRasterLayers(context.project()):
            expression = self.mappedNameToLayer(lyr, expression, layersDict, context)

        # create the list of layers to be passed as inputs to RasterCalculaltor
        # at this phase expression has been modified to match available layers
        # in the current scope
        entries = []
        for name, lyr in layersDict.items():
            for n in range(lyr.bandCount()):
                ref = '{:s}@{:d}'.format(name, n + 1)

                if ref in expression:
                    entry = QgsRasterCalculatorEntry()
                    entry.ref = ref
                    entry.raster = lyr
                    entry.bandNumber = n + 1
                    entries.append(entry)

        output = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        width = math.floor((bbox.xMaximum() - bbox.xMinimum()) / cellsize)
        height = math.floor((bbox.yMaximum() - bbox.yMinimum()) / cellsize)
        driverName = GdalUtils.getFormatShortNameFromFilename(output)

        calc = QgsRasterCalculator(expression,
                                   output,
                                   driverName,
                                   bbox,
                                   crs,
                                   width,
                                   height,
                                   entries)

        res = calc.processCalculation(feedback)
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

    def mappedNameToLayer(self, lyr, expression, layersDict, context):
        '''Try to identify if a real layer is mapped in the expression with a symbolic name.'''

        nameToMap = lyr.source()

        # check if nameToMap is a file
        # TODO: what about URI eg for a COG?
        if os.path.isfile(nameToMap):
            # get only the name without extension and path of the file
            nameToMap = os.path.splitext(os.path.basename(nameToMap))[0]

        # check for layers directly added in the expression
        if (nameToMap + "@") in expression:
            layersDict[nameToMap] = lyr

        # get "algorithm_inputs" scope of the expressionContext related
        # with mapped variables
        indexOfScope = context.expressionContext().indexOfScope("algorithm_inputs")
        if indexOfScope >= 0:
            expContextAlgInputsScope = context.expressionContext().scope(indexOfScope)

            # check for the layers that are mapped as input in a model
            #  to do this check in the latest scope all passed variables
            # to look for a variable that is a layer or a string filename ç
            # to a layer
            varDescription = None
            for varName in expContextAlgInputsScope.variableNames():

                layerInContext = expContextAlgInputsScope.variable(varName)

                if not isinstance(layerInContext, str) and not isinstance(layerInContext, QgsMapLayer):
                    continue

                if isinstance(layerInContext, QgsMapLayer) and nameToMap not in layerInContext.source():
                    continue

                varDescription = expContextAlgInputsScope.description(varName)

                # because there can be variable with None or "" description
                # then skip them
                if not varDescription:
                    continue

                # check if it's description starts with Output as in:
                #    Output 'Output' from algorithm 'calc1'
                # as set in https://github.com/qgis/QGIS/blob/master/src/core/processing/models/qgsprocessingmodelalgorithm.cpp#L516
                # but var in expression is called simply
                #    'Output' from algorithm 'calc1'

                # get the translatin string to use to parse the description
                # HAVE to use the same translated string as in
                # https://github.com/qgis/QGIS/blob/master/src/core/processing/models/qgsprocessingmodelalgorithm.cpp#L516
                translatedDesc = self.tr("Output '%1' from algorithm '%2'")
                elementZero = translatedDesc.split(" ")[0] # For english the string result should be "Output"

                elements = varDescription.split(" ")
                if len(elements) > 1 and elements[0] == elementZero:
                    # remove heading QObject.tr"Output ") string. Note adding a space at the end of elementZero!
                    varDescription = varDescription[len(elementZero) + 1:]

                # check if cleaned varDescription is present in the expression
                # if not skip it
                if (varDescription + "@") not in expression:
                    continue

                # !!!found!!! => substitute in expression
                # and add in the list of layers that will be passed to raster calculator
                nameToMap = varName
                new = "{}@".format(nameToMap)
                old = "{}@".format(varDescription)
                expression = expression.replace(old, new)

                layersDict[nameToMap] = lyr

        # need return the modified expression because it's not a reference
        return expression
