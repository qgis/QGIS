# -*- coding: utf-8 -*-

"""
***************************************************************************
    Datasources2Vrt.py
    ---------------------
    Date                 : May 2015
    Copyright            : (C) 2015 by Luigi Pirelli
    Email                : luipir at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Luigi Pirelli'
__date__ = 'May 2015'
__copyright__ = '(C) 2015, Luigi Pirelli'

import html

from qgis.core import (QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputString
                       )
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils


class Datasources2Vrt(QgisAlgorithm):
    INPUT = 'INPUT'
    UNIONED = 'UNIONED'
    OUTPUT = 'OUTPUT'
    VRT_STRING = 'VRT_STRING'

    def group(self):
        return self.tr('Vector general')

    def groupId(self):
        return 'vectorgeneral'

    def name(self):
        return 'buildvirtualvector'

    def displayName(self):
        return self.tr('Build virtual vector')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterMultipleLayers(self.INPUT,
                                                               self.tr('Input datasources'),
                                                               QgsProcessing.TypeVector))
        self.addParameter(QgsProcessingParameterBoolean(self.UNIONED,
                                                        self.tr('Create "unioned" VRT'),
                                                        defaultValue=False))

        class ParameterVectorVrtDestination(QgsProcessingParameterVectorDestination):

            def __init__(self, name, description):
                super().__init__(name, description)

            def clone(self):
                copy = ParameterVectorVrtDestination(self.name(), self.description())
                return copy

            def type(self):
                return 'vrt_vector_destination'

            def defaultFileExtension(self):
                return 'vrt'

        self.addParameter(ParameterVectorVrtDestination(self.OUTPUT,
                                                        self.tr('Virtual vector')))
        self.addOutput(QgsProcessingOutputString(self.VRT_STRING,
                                                 self.tr('Virtual string')))

    def processAlgorithm(self, parameters, context, feedback):
        input_layers = self.parameterAsLayerList(parameters, self.INPUT, context)
        unioned = self.parameterAsBoolean(parameters, self.UNIONED, context)
        vrtPath = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)

        vrt = '<OGRVRTDataSource>'
        if unioned:
            vrt += '<OGRVRTUnionLayer name="UnionedLayer">'

        total = 100.0 / len(input_layers) if input_layers else 0
        for current, layer in enumerate(input_layers):
            if feedback.isCanceled():
                break

            basePath = GdalUtils.ogrConnectionStringFromLayer(layer)
            layerName = GdalUtils.ogrLayerName(layer.source())

            vrt += '<OGRVRTLayer name="{}">'.format(html.escape(layerName, True))
            vrt += '<SrcDataSource>{}</SrcDataSource>'.format(html.escape(basePath, True))
            vrt += '<SrcLayer>{}</SrcLayer>'.format(html.escape(layerName, True))
            vrt += '</OGRVRTLayer>'

            feedback.setProgress(int(current * total))

        if unioned:
            vrt += '</OGRVRTUnionLayer>'
        vrt += '</OGRVRTDataSource>'

        with open(vrtPath, 'w', encoding='utf-8') as f:
            f.write(vrt)

        return {self.OUTPUT: vrtPath, self.VRT_STRING: vrt}
