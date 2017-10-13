# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdal2tiles.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingOutputFolder,
                       QgsProcessingParameterFolderDestination)
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils
from processing.tools.system import isWindows


class gdal2tiles(GdalAlgorithm):

    INPUT = 'INPUT'
    PROFILE = 'PROFILE'
    RESAMPLING = 'RESAMPLING'
    ZOOM = 'ZOOM'
    SOURCE_CRS = 'SOURCE_CRS'
    NODATA = 'NODATA'
    KML = 'KML'
    NO_KML = 'NO_KML'
    URL = 'URL'
    VIEWER = 'VIEWER'
    TITLE = 'TITLE'
    COPYRIGHT = 'COPYRIGHT'
    GOOGLE_KEY = 'GOOGLE_KEY'
    BING_KEY = 'BING_KEY'
    RESUME = 'RESUME'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.profiles = ((self.tr('Mercator'), 'mercator'),
                         (self.tr('Geodetic'), 'geodetic'),
                         (self.tr('Raster'), 'raster'))

        self.methods = ((self.tr('Average'), 'average'),
                        (self.tr('Nearest neighbour'), 'near'),
                        (self.tr('Bilinear'), 'bilinear'),
                        (self.tr('Cubic'), 'cubic'),
                        (self.tr('Cubic spline'), 'cubicspline'),
                        (self.tr('Lanczos windowed sinc'), 'lanczos'),
                        (self.tr('Antialias'), 'antialias'))

        self.viewers = ((self.tr('All'), 'all'),
                        (self.tr('GoogleMaps'), 'google'),
                        (self.tr('OpenLayers'), 'openlayers'),
                        (self.tr('Leaflet'), 'leaflet'),
                        (self.tr('None'), 'none'))

        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT, self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterEnum(self.PROFILE,
                                                     self.tr('Tile cutting profile'),
                                                     options=[i[0] for i in self.profiles],
                                                     allowMultiple=False,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterString(self.ZOOM,
                                                       self.tr('Zoom levels to render'),
                                                       defaultValue='',
                                                       optional=True))
        self.addParameter(QgsProcessingParameterEnum(self.VIEWER,
                                                     self.tr('Web viewer to generate'),
                                                     options=[i[0] for i in self.viewers],
                                                     allowMultiple=False,
                                                     defaultValue=0))
        self.addParameter(QgsProcessingParameterString(self.TITLE,
                                                       self.tr('Title of the map'),
                                                       optional=True))
        self.addParameter(QgsProcessingParameterString(self.COPYRIGHT,
                                                       self.tr('Copyright of the map'),
                                                       optional=True))

        params = []
        params.append(QgsProcessingParameterEnum(self.RESAMPLING,
                                                 self.tr('Resampling method'),
                                                 options=[i[0] for i in self.methods],
                                                 allowMultiple=False,
                                                 defaultValue=0))
        params.append(QgsProcessingParameterCrs(self.SOURCE_CRS,
                                                self.tr('The spatial reference system used for the source input data'),
                                                optional=True))
        params.append(QgsProcessingParameterNumber(self.NODATA,
                                                   self.tr('Transparency value to assign to the input data'),
                                                   type=QgsProcessingParameterNumber.Double,
                                                   defaultValue=0,
                                                   optional=True))
        params.append(QgsProcessingParameterString(self.URL,
                                                   self.tr('URL address where the generated tiles are going to be published'),
                                                   optional=True))
        params.append(QgsProcessingParameterString(self.GOOGLE_KEY,
                                                   self.tr('Google Maps API key (http://code.google.com/apis/maps/signup.html)'),
                                                   optional=True))
        params.append(QgsProcessingParameterString(self.BING_KEY,
                                                   self.tr('Bing Maps API key (https://www.bingmapsportal.com/)'),
                                                   optional=True))
        params.append(QgsProcessingParameterBoolean(self.RESUME,
                                                    self.tr('Generate only missing files'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.KML,
                                                    self.tr('Generate KML for Google Earth'),
                                                    defaultValue=False))
        params.append(QgsProcessingParameterBoolean(self.NO_KML,
                                                    self.tr('Avoid automatic generation of KML files for EPSG:4326'),
                                                    defaultValue=False))
        for param in params:
            param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            self.addParameter(param)

        self.addParameter(QgsProcessingParameterFolderDestination(self.OUTPUT,
                                                                  self.tr('Output directory')))

        self.addOutput(QgsProcessingOutputFolder(self.OUTPUT, self.tr('Output directory')))

    def name(self):
        return 'gdal2tiles'

    def displayName(self):
        return self.tr('gdal2tiles')

    def group(self):
        return self.tr('Raster miscellaneous')

    def getConsoleCommands(self, parameters, context, feedback):
        arguments = []

        arguments.append('-p')
        arguments.append(self.profiles[self.parameterAsEnum(parameters, self.PROFILE, context)][1])

        zoom = self.parameterAsString(parameters, self.ZOOM, context)
        if zoom:
            arguments.append('-z')
            arguments.append(str(zoom))

        arguments.append('-w')
        arguments.append(self.viewers[self.parameterAsEnum(parameters, self.VIEWER, context)][1])

        title = self.parameterAsString(parameters, self.TITLE, context)
        if title:
            arguments.append('-t')
            arguments.append(title)

        copying = self.parameterAsString(parameters, self.COPYRIGHT, context)
        if copying:
            arguments.append('-c')
            arguments.append(copying)

        arguments.append('-r')
        arguments.append(self.methods[self.parameterAsEnum(parameters, self.RESAMPLING, context)][1])

        crs = self.parameterAsCrs(parameters, self.SOURCE_CRS, context)
        if crs.isValid():
            arguments.append('-s')
            arguments.append(crs.authid())

        nodata = self.parameterAsDouble(parameters, self.NODATA, context)
        if nodata:
            arguments.append('-a')
            arguments.append(str(nodata))

        url = self.parameterAsString(parameters, self.URL, context)
        if url:
            arguments.append('-u')
            arguments.append(url)

        key = self.parameterAsString(parameters, self.GOOGLE_KEY, context)
        if key:
            arguments.append('-g')
            arguments.append(key)

        key = self.parameterAsString(parameters, self.BING_KEY, context)
        if key:
            arguments.append('-b')
            arguments.append(key)

        if self.parameterAsBool(parameters, self.RESUME, context):
            arguments.append('-e')

        if self.parameterAsBool(parameters, self.KML, context):
            arguments.append('-k')

        if self.parameterAsBool(parameters, self.NO_KML, context):
            arguments.append('-n')

        inLayer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        arguments.append(inLayer.source())
        arguments.append(self.parameterAsString(parameters, self.OUTPUT, context))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal2tiles.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal2tiles.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
