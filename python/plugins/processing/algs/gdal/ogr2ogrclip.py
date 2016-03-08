# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogrclip.py
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

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.system import isWindows
from processing.tools.vector import ogrConnectionString, ogrLayerName


class Ogr2OgrClip(GdalAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    CLIP_LAYER = 'CLIP_LAYER'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip vectors by polygon')
        self.group, self.i18n_group = self.trAlgorithm('[OGR] Geoprocessing')

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterVector(self.CLIP_LAYER,
                                          self.tr('Clip layer'), [ParameterVector.VECTOR_TYPE_POLYGON], False))
        self.addParameter(ParameterString(self.OPTIONS,
                                          self.tr('Additional creation options'), '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Clipped (polygon)')))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = ogrConnectionString(inLayer)[1:-1]
        clipLayer = self.getParameterValue(self.CLIP_LAYER)
        ogrClipLayer = ogrConnectionString(clipLayer)[1:-1]

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append('-clipsrc')
        arguments.append(ogrClipLayer)
        arguments.append("-clipsrclayer")
        arguments.append(ogrLayerName(clipLayer))
        if len(options) > 0:
            arguments.append(options)

        arguments.append(output)
        arguments.append(ogrLayer)
        arguments.append(ogrLayerName(inLayer))

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        return commands

    def commandName(self):
        return "ogr2ogr"
