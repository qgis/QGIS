# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogr.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputVector

from processing.tools.system import isWindows

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

FORMATS = [
    'ESRI Shapefile',
    'GeoJSON',
    'GeoRSS',
    'SQLite',
    'GMT',
    'MapInfo File',
    'INTERLIS 1',
    'INTERLIS 2',
    'GML',
    'Geoconcept',
    'DXF',
    'DGN',
    'CSV',
    'BNA',
    'S57',
    'KML',
    'GPX',
    'PGDump',
    'GPSTrackMaker',
    'ODS',
    'XLSX',
    'PDF',
]

EXTS = [
    '.shp',
    '.geojson',
    '.xml',
    '.sqlite',
    '.gmt',
    '.tab',
    '.ili',
    '.ili',
    '.gml',
    '.txt',
    '.dxf',
    '.dgn',
    '.csv',
    '.bna',
    '.000',
    '.kml',
    '.gpx',
    '.pgdump',
    '.gtm',
    '.ods',
    '.xlsx',
    '.pdf',
]


class Ogr2Ogr(OgrAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    FORMAT = 'FORMAT'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name = 'Convert format'
        self.group = '[OGR] Conversion'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterSelection(self.FORMAT,
            self.tr('Destination Format'), FORMATS))
        self.addParameter(ParameterString(self.OPTIONS,
            self.tr('Creation options'), '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Converted')))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)[1:-1]

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        formatIdx = self.getParameterValue(self.FORMAT)
        outFormat = FORMATS[formatIdx]
        ext = EXTS[formatIdx]
        if not outFile.endswith(ext):
            outFile += ext
            output.value = outFile

        output = self.ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        if outFormat == 'SQLite' and os.path.isfile(output):
            os.remove(output)

        arguments = []
        arguments.append('-f')
        arguments.append(outFormat)
        if len(options) > 0:
            arguments.append(options)

        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(self.ogrLayerName(inLayer))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        return commands

    def commandName(self):
        return "ogr2ogr"