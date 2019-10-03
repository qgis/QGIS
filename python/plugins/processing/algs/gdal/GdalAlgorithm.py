# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import re

from qgis.PyQt.QtCore import QUrl, QCoreApplication

from qgis.core import (QgsApplication,
                       QgsVectorFileWriter,
                       QgsProcessingFeatureSourceDefinition,
                       QgsProcessingAlgorithm,
                       QgsProcessingContext,
                       QgsProcessingFeedback,
                       QgsProviderRegistry,
                       QgsDataSourceUri)

from processing.algs.gdal.GdalAlgorithmDialog import GdalAlgorithmDialog
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class GdalAlgorithm(QgsProcessingAlgorithm):

    def __init__(self):
        super().__init__()
        self.output_values = {}

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGdal.svg")

    def tags(self):
        return ['ogr', 'gdal', self.commandName()]

    def svgIconPath(self):
        return QgsApplication.iconPath("providerGdal.svg")

    def createInstance(self, config={}):
        return self.__class__()

    def createCustomParametersWidget(self, parent):
        return GdalAlgorithmDialog(self, parent=parent)

    def flags(self):
        return QgsProcessingAlgorithm.FlagSupportsBatch # cannot cancel!

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        return None

    def getOgrCompatibleSource(self, parameter_name, parameters, context, feedback, executing):
        """
        Interprets a parameter as an OGR compatible source and layer name
        :param executing:
        """
        if not executing and parameter_name in parameters and isinstance(parameters[parameter_name], QgsProcessingFeatureSourceDefinition):
            # if not executing, then we throw away all 'selected features only' settings
            # since these have no meaning for command line gdal use, and we don't want to force
            # an export of selected features only to a temporary file just to show the command!
            parameters = {parameter_name: parameters[parameter_name].source}

        input_layer = self.parameterAsVectorLayer(parameters, parameter_name, context)
        ogr_data_path = None
        ogr_layer_name = None
        if input_layer is None or input_layer.dataProvider().name() == 'memory':
            if executing:
                # parameter is not a vector layer - try to convert to a source compatible with OGR
                # and extract selection if required
                ogr_data_path = self.parameterAsCompatibleSourceLayerPath(parameters, parameter_name, context,
                                                                          QgsVectorFileWriter.supportedFormatExtensions(),
                                                                          QgsVectorFileWriter.supportedFormatExtensions()[0],
                                                                          feedback=feedback)
                ogr_layer_name = GdalUtils.ogrLayerName(ogr_data_path)
            else:
                #not executing - don't waste time converting incompatible sources, just return dummy strings
                #for the command preview (since the source isn't compatible with OGR, it has no meaning anyway and can't
                #be run directly in the command line)
                ogr_data_path = 'path_to_data_file'
                ogr_layer_name = 'layer_name'
        elif input_layer.dataProvider().name() == 'ogr':
            if executing and isinstance(parameters[parameter_name], QgsProcessingFeatureSourceDefinition) and parameters[parameter_name].selectedFeaturesOnly:
                # parameter is a vector layer, with OGR data provider
                # so extract selection if required
                ogr_data_path = self.parameterAsCompatibleSourceLayerPath(parameters, parameter_name, context,
                                                                          QgsVectorFileWriter.supportedFormatExtensions(),
                                                                          feedback=feedback)
                parts = QgsProviderRegistry.instance().decodeUri('ogr', ogr_data_path)
                ogr_data_path = parts['path']
                if 'layerName' in parts and parts['layerName']:
                    ogr_layer_name = parts['layerName']
                else:
                    ogr_layer_name = GdalUtils.ogrLayerName(ogr_data_path)
            else:
                #either not using the selection, or
                #not executing - don't worry about 'selected features only' handling. It has no meaning
                #for the command line preview since it has no meaning outside of a QGIS session!
                ogr_data_path = GdalUtils.ogrConnectionStringAndFormatFromLayer(input_layer)[0]
                ogr_layer_name = GdalUtils.ogrLayerName(input_layer.dataProvider().dataSourceUri())
        elif input_layer.dataProvider().name().lower() == 'wfs':
            uri = QgsDataSourceUri(input_layer.source())
            baseUrl = uri.param('url').split('?')[0]
            ogr_data_path = "WFS:{}".format(baseUrl)
            ogr_layer_name = uri.param('typename')
        else:
            # vector layer, but not OGR - get OGR compatible path
            # TODO - handle "selected features only" mode!!
            ogr_data_path = GdalUtils.ogrConnectionStringFromLayer(input_layer)
            ogr_layer_name = GdalUtils.ogrLayerName(input_layer.dataProvider().dataSourceUri())
        return ogr_data_path, ogr_layer_name

    def setOutputValue(self, name, value):
        self.output_values[name] = value

    def processAlgorithm(self, parameters, context, feedback):
        commands = self.getConsoleCommands(parameters, context, feedback, executing=True)
        GdalUtils.runGdal(commands, feedback)

        # auto generate outputs
        results = {}
        for o in self.outputDefinitions():
            if o.name() in parameters:
                results[o.name()] = parameters[o.name()]
        for k, v in self.output_values.items():
            results[k] = v

        return results

    def commandName(self):
        parameters = {}
        for param in self.parameterDefinitions():
            parameters[param.name()] = "1"
        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        name = self.getConsoleCommands(parameters, context, feedback, executing=False)[0]
        if name.endswith(".py"):
            name = name[:-3]
        return name

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)
