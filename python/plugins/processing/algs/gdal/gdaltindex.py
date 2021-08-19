# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdaltindex.py
    ---------------------
    Date                 : February 2015
    Copyright            : (C) 2015 by Pedro Venancio
    Email                : pedrongvenancio at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Pedro Venancio'
__date__ = 'February 2015'
__copyright__ = '(C) 2015, Pedro Venancio'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsMapLayer,
                       QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingException,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterVectorDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class gdaltindex(GdalAlgorithm):
    LAYERS = 'LAYERS'
    PATH_FIELD_NAME = 'PATH_FIELD_NAME'
    ABSOLUTE_PATH = 'ABSOLUTE_PATH'
    PROJ_DIFFERENCE = 'PROJ_DIFFERENCE'
    TARGET_CRS = 'TARGET_CRS'
    CRS_FIELD_NAME = 'CRS_FIELD_NAME'
    CRS_FORMAT = 'CRS_FORMAT'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.formats = ((self.tr('Auto'), 'AUTO'),
                        (self.tr('Well-known text (WKT)'), 'WKT'),
                        (self.tr('EPSG'), 'EPSG'),
                        (self.tr('Proj.4'), 'PROJ'))

        self.addParameter(QgsProcessingParameterMultipleLayers(self.LAYERS,
                                                               self.tr('Input files'),
                                                               QgsProcessing.TypeRaster))
        self.addParameter(QgsProcessingParameterString(self.PATH_FIELD_NAME,
                                                       self.tr('Field name to hold the file path to the indexed rasters'),
                                                       defaultValue='location'))
        self.addParameter(QgsProcessingParameterBoolean(self.ABSOLUTE_PATH,
                                                        self.tr('Store absolute path to the indexed rasters'),
                                                        defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.PROJ_DIFFERENCE,
                                                        self.tr('Skip files with different projection reference'),
                                                        defaultValue=False))

        target_crs_param = QgsProcessingParameterCrs(self.TARGET_CRS,
                                                     self.tr('Transform geometries to the given CRS'),
                                                     optional=True)
        target_crs_param.setFlags(target_crs_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(target_crs_param)

        crs_field_param = QgsProcessingParameterString(self.CRS_FIELD_NAME,
                                                       self.tr('The name of the field to store the SRS of each tile'),
                                                       optional=True)
        crs_field_param.setFlags(crs_field_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(crs_field_param)

        crs_format_param = QgsProcessingParameterEnum(self.CRS_FORMAT,
                                                      self.tr('The format in which the CRS of each tile must be written'),
                                                      options=[i[0] for i in self.formats],
                                                      allowMultiple=False,
                                                      defaultValue=0)
        crs_format_param.setFlags(crs_format_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(crs_format_param)

        self.addParameter(QgsProcessingParameterVectorDestination(self.OUTPUT,
                                                                  self.tr('Tile index'),
                                                                  QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'tileindex'

    def displayName(self):
        return self.tr('Tile index')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'tiles.png'))

    def commandName(self):
        return 'gdaltindex'

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        input_layers = self.parameterAsLayerList(parameters, self.LAYERS, context)
        crs_field = self.parameterAsString(parameters, self.CRS_FIELD_NAME, context)
        crs_format = self.parameterAsEnum(parameters, self.CRS_FORMAT, context)
        target_crs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)

        outFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        self.setOutputValue(self.OUTPUT, outFile)
        output, outFormat = GdalUtils.ogrConnectionStringAndFormat(outFile, context)

        arguments = [
            '-tileindex',
            self.parameterAsString(parameters, self.PATH_FIELD_NAME, context),
        ]

        if self.parameterAsBoolean(parameters, self.ABSOLUTE_PATH, context):
            arguments.append('-write_absolute_path')

        if self.parameterAsBoolean(parameters, self.PROJ_DIFFERENCE, context):
            arguments.append('-skip_different_projection')

        if crs_field:
            arguments.append('-src_srs_name {}'.format(crs_field))

        if crs_format:
            arguments.append('-src_srs_format {}'.format(self.formats[crs_format][1]))

        if target_crs.isValid():
            arguments.append('-t_srs')
            arguments.append(GdalUtils.gdal_crs_string(target_crs))

        if outFormat:
            arguments.append('-f {}'.format(outFormat))

        arguments.append(output)

        # Always write input files to a text file in case there are many of them and the
        # length of the command will be longer then allowed in command prompt
        list_file = GdalUtils.writeLayerParameterToTextFile(filename='tile_index_files.txt', alg=self, parameters=parameters, parameter_name=self.LAYERS, context=context, quote=True, executing=executing)
        arguments.append('--optfile')
        arguments.append(list_file)

        return [self.commandName(), GdalUtils.escapeAndJoin(arguments)]
