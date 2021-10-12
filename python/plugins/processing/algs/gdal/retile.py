# -*- coding: utf-8 -*-

"""
***************************************************************************
    retile.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

from qgis.core import (QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows


class retile(GdalAlgorithm):
    INPUT = 'INPUT'
    TILE_SIZE_X = 'TILE_SIZE_X'
    TILE_SIZE_Y = 'TILE_SIZE_Y'
    OVERLAP = 'OVERLAP'
    LEVELS = 'LEVELS'

    SOURCE_CRS = 'SOURCE_CRS'
    FORMAT = 'FORMAT'
    RESAMPLING = 'RESAMPLING'
    OPTIONS = 'OPTIONS'
    EXTRA = 'EXTRA'
    DATA_TYPE = 'DATA_TYPE'
    DELIMITER = 'DELIMITER'
    ONLY_PYRAMIDS = 'ONLY_PYRAMIDS'
    DIR_FOR_ROW = 'DIR_FOR_ROW'
    OUTPUT = 'OUTPUT'
    OUTPUT_CSV = 'OUTPUT_CSV'

    TYPES = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64', 'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = ((self.tr('Nearest Neighbour'), 'near'),
                        (self.tr('Bilinear'), 'bilinear'),
                        (self.tr('Cubic'), 'cubic'),
                        (self.tr('Cubic Spline'), 'cubicspline'),
                        (self.tr('Lanczos Windowed Sinc'), 'lanczos'),)

        self.addParameter(QgsProcessingParameterMultipleLayers(self.INPUT,
                                                               self.tr('Input files'),
                                                               QgsProcessing.TypeRaster))
        self.addParameter(QgsProcessingParameterNumber(self.TILE_SIZE_X,
                                                       self.tr('Tile width'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=256))
        self.addParameter(QgsProcessingParameterNumber(self.TILE_SIZE_Y,
                                                       self.tr('Tile height'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=256))
        self.addParameter(QgsProcessingParameterNumber(self.OVERLAP,
                                                       self.tr('Overlap in pixels between consecutive tiles'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.LEVELS,
                                                       self.tr('Number of pyramids levels to build'),
                                                       type=QgsProcessingParameterNumber.Integer,
                                                       minValue=0,
                                                       defaultValue=1))

        params = [
            QgsProcessingParameterCrs(self.SOURCE_CRS,
                                      self.tr('Source coordinate reference system'),
                                      optional=True,
                                      ),
            QgsProcessingParameterEnum(self.RESAMPLING,
                                       self.tr('Resampling method'),
                                       options=[i[0] for i in self.methods],
                                       allowMultiple=False,
                                       defaultValue=0),
            QgsProcessingParameterString(self.DELIMITER,
                                         self.tr('Column delimiter used in the CSV file'),
                                         defaultValue=';',
                                         optional=True)

        ]
        options_param = QgsProcessingParameterString(self.OPTIONS,
                                                     self.tr('Additional creation options'),
                                                     defaultValue='',
                                                     optional=True)
        options_param.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.gdal.ui.RasterOptionsWidget.RasterOptionsWidgetWrapper'}})
        params.append(options_param)

        params.append(QgsProcessingParameterString(self.EXTRA,
                                                   self.tr('Additional command-line parameters'),
                                                   defaultValue=None,
                                                   optional=True))

        params.append(QgsProcessingParameterEnum(self.DATA_TYPE,
                                                 self.tr('Output data type'),
                                                 self.TYPES,
                                                 allowMultiple=False,
                                                 defaultValue=5))

        params.append(QgsProcessingParameterBoolean(self.ONLY_PYRAMIDS,
                                                    self.tr('Build only the pyramids'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.DIR_FOR_ROW,
                                                    self.tr('Use separate directory for each tiles row'),
                                                    defaultValue=False))

        for param in params:
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(param)

        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT,
                                                                  self.tr('Output directory')))

        output_csv_param = QgsProcessingParameterFileDestination(self.OUTPUT_CSV,
                                                                 self.tr('CSV file containing the tile(s) georeferencing information'),
                                                                 'CSV files (*.csv)',
                                                                 optional=True)
        output_csv_param.setCreateByDefault(False)
        self.addParameter(output_csv_param)

    def name(self):
        return 'retile'

    def displayName(self):
        return self.tr('Retile')

    def group(self):
        return self.tr('Raster miscellaneous')

    def groupId(self):
        return 'rastermiscellaneous'

    def commandName(self):
        return "gdal_retile"

    def getConsoleCommands(self, parameters, context, feedback, executing=True):
        arguments = [
            '-ps',
            str(self.parameterAsInt(parameters, self.TILE_SIZE_X, context)),
            str(self.parameterAsInt(parameters, self.TILE_SIZE_Y, context)),

            '-overlap',
            str(self.parameterAsInt(parameters, self.OVERLAP, context)),

            '-levels',
            str(self.parameterAsInt(parameters, self.LEVELS, context))
        ]

        crs = self.parameterAsCrs(parameters, self.SOURCE_CRS, context)
        if crs.isValid():
            arguments.append('-s_srs')
            arguments.append(GdalUtils.gdal_crs_string(crs))

        arguments.append('-r')
        arguments.append(self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1])

        arguments.append('-ot')
        arguments.append(self.TYPES[self.parameterAsEnum(parameters, self.DATA_TYPE, context)])

        options = self.parameterAsString(parameters, self.OPTIONS, context)
        if options:
            arguments.extend(GdalUtils.parseCreationOptions(options))

        if self.EXTRA in parameters and parameters[self.EXTRA] not in (None, ''):
            extra = self.parameterAsString(parameters, self.EXTRA, context)
            arguments.append(extra)

        if self.parameterAsBoolean(parameters, self.DIR_FOR_ROW, context):
            arguments.append('-useDirForEachRow')

        if self.parameterAsBoolean(parameters, self.ONLY_PYRAMIDS, context):
            arguments.append('-pyramidOnly')

        csvFile = self.parameterAsFileOutput(parameters, self.OUTPUT_CSV, context)
        if csvFile:
            arguments.append('-csv')
            arguments.append(csvFile)
            delimiter = self.parameterAsString(parameters, self.DELIMITER, context)
            if delimiter:
                arguments.append('-csvDelim')
                arguments.append(delimiter)

        arguments.append('-targetDir')
        arguments.append(self.parameterAsString(parameters, self.OUTPUT, context))

        layers = [l.source() for l in self.parameterAsLayerList(parameters, self.INPUT, context)]
        arguments.extend(layers)

        return [self.commandName() + ('.bat' if isWindows() else '.py'), GdalUtils.escapeAndJoin(arguments)]
