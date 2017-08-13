# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrclipextent.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterExtent
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows


class Ogr2OgrClipExtent(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    CLIP_EXTENT = 'CLIP_EXTENT'
    OPTIONS = 'OPTIONS'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer')))
        self.addParameter(ParameterExtent(self.CLIP_EXTENT,
                                          self.tr('Clip extent')))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'), '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Clipped (extent)')))

    def name(self):
        return 'clipvectorsbyextent'

    def displayName(self):
        return self.tr('Clip vectors by extent')

    def group(self):
        return self.tr('Vector geoprocessing')

    def getConsoleCommands(self, parameters, context, feedback):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = GdalUtils.ogrConnectionString(inLayer, context)[1:-1]
        clipExtent = self.getParameterValue(self.CLIP_EXTENT)
        if not clipExtent:
            clipExtent = QgsProcessingUtils.combineLayerExtents([inLayer])

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = GdalUtils.ogrConnectionString(outFile, context)
        options = str(self.getParameterValue(self.OPTIONS))

        arguments = []
        regionCoords = clipExtent.split(',')
        arguments.append('-spat')
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[2])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[3])
        arguments.append('-clipsrc spat_extent')

        if options is not None and len(options.strip()) > 0:
            arguments.append(options)

        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(GdalUtils.ogrLayerName(inLayer))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        return commands

    def commandName(self):
        return "ogr2ogr"
