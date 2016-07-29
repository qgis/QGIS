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

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterBoolean
from processing.core.outputs import OutputDirectory
from processing.tools.system import isWindows
from processing.algs.gdal.GdalUtils import GdalUtils


class gdal2tiles(GdalAlgorithm):

    INPUT = 'INPUT'
    PROFILE = 'PROFILE'
    RESAMPLING = 'RESAMPLING'
    ZOOM = 'ZOOM'
    S_SRS = 'S_SRS'
    OUTPUTDIR = 'OUTPUTDIR'
    RESUME = 'RESUME'
    NODATA = 'NODATA'
    FORCEKML = 'FORCEKML'
    NOKML = 'NOKML'
    URL = 'URL'
    WEBVIEWER = 'WEBVIEWER'
    TITLE = 'TITLE'
    COPYRIGHT = 'COPYRIGHT'
    GOOGLEKEY = 'GOOGLEKEY'
    BINGKEY = 'BINGKEY'

    PROFILES = ['mercator', 'geodetic', 'raster']
    RESAMPLINGS = ['average', 'near', 'bilinear', 'cubic', 'cubicspline', 'lanczos', 'antialias']
    WEBVIEWERS = ['all', 'google', 'openlayers', 'leaflet', 'none']

    def commandLineName(self):
        return "gdalogr:gdal2tiles"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('gdal2tiles')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Miscellaneous')

        # Required parameters
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer')))

        # Advanced parameters
        params = []
        params.append(ParameterSelection(self.PROFILE,
                                         self.tr('Tile cutting profile'),
                                         self.PROFILES, 0, False, True))
        params.append(ParameterSelection(self.RESAMPLING,
                                         self.tr('Resampling method'),
                                         self.RESAMPLINGS, 0, False, True))
        params.append(ParameterCrs(self.S_SRS,
                                   self.tr('The spatial reference system used for the source input data'),
                                   None, True))
        params.append(ParameterString(self.ZOOM,
                                      self.tr('Zoom levels to render'),
                                      None, False, True))
        params.append(ParameterBoolean(self.RESUME,
                                       self.tr('Resume mode, generate only missing files'),
                                       False, True))
        params.append(ParameterString(self.NODATA,
                                      self.tr('NODATA transparency value to assign to the input data'),
                                      None, False, True))
        params.append(ParameterBoolean(self.FORCEKML,
                                       self.tr('Generate KML for Google Earth - default for "geodetic" profile and "raster" in EPSG:4326'),
                                       False, True))
        params.append(ParameterBoolean(self.NOKML,
                                       self.tr('Avoid automatic generation of KML files for EPSG:4326'),
                                       False, True))
        params.append(ParameterString(self.URL,
                                      self.tr('URL address where the generated tiles are going to be published'),
                                      None, False, True))
        params.append(ParameterSelection(self.WEBVIEWER,
                                         self.tr('Web viewer to generate'),
                                         self.WEBVIEWERS, 0, False, True))
        params.append(ParameterString(self.TITLE,
                                      self.tr('Title of the map'),
                                      None, False, True))
        params.append(ParameterString(self.COPYRIGHT,
                                      self.tr('Copyright for the map'),
                                      None, False, True))
        params.append(ParameterString(self.GOOGLEKEY,
                                      self.tr('Google Maps API key from http://code.google.com/apis/maps/signup.html'),
                                      None, False, True))
        params.append(ParameterString(self.BINGKEY,
                                      self.tr('Bing Maps API key from https://www.bingmapsportal.com/'),
                                      None, False, True))

        for param in params:
            param.isAdvanced = True
            self.addParameter(param)

        self.addOutput(OutputDirectory(self.OUTPUTDIR,
                                       self.tr('The directory where the tile result is created')))

    def getConsoleCommands(self):

        arguments = []

        if self.getParameterValue(self.PROFILE):
            arguments.append('-p')
            arguments.append(self.PROFILES[self.getParameterValue(self.PROFILE)])

        if self.getParameterValue(self.RESAMPLING):
            arguments.append('-r')
            arguments.append(self.RESAMPLINGS[self.getParameterValue(self.RESAMPLING)])

        ssrs = unicode(self.getParameterValue(self.S_SRS))
        if len(ssrs) > 0:
            arguments.append('-s')
            arguments.append(ssrs)

        if self.getParameterValue(self.ZOOM):
            arguments.append('-z')
            arguments.append(unicode(self.getParameterValue(self.ZOOM)))

        if self.getParameterValue(self.RESUME):
            arguments.append('-e')

        if self.getParameterValue(self.NODATA):
            arguments.append('-a')
            arguments.append(unicode(self.getParameterValue(self.NODATA)))

        # KML arguments
        if self.getParameterValue(self.FORCEKML):
            arguments.append('-k')

        if self.getParameterValue(self.NOKML):
            arguments.append('-n')

        if self.getParameterValue(self.URL):
            arguments.append('-u')
            arguments.append(unicode(self.getParameterValue(self.URL)))

        # Web viewer arguments
        if self.getParameterValue(self.WEBVIEWER):
            arguments.append('-w')
            arguments.append(self.WEBVIEWERS[self.getParameterValue(self.WEBVIEWER)])

        parameters = {self.TITLE: '-t', self.COPYRIGHT: '-c',
                      self.GOOGLEKEY: '-g', self.BINGKEY: '-b'}
        for arg, parameter in parameters.iteritems():
            if self.getParameterValue(arg):
                arguments.append(parameter)
                arguments.append(self.getParameterValue(arg))

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(self.getOutputValue(self.OUTPUTDIR))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'gdal2tiles.bat',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['gdal2tiles.py',
                        GdalUtils.escapeAndJoin(arguments)]

        return commands
