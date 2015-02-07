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

from processing.tools.system import isWindows

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrClip(OgrAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    CLIP_LAYER = 'CLIP_LAYER'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name = 'Clip vectors by polygon'
        self.group = '[OGR] Geoprocessing'

        self.addParameter(ParameterVector(self.INPUT_LAYER,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterVector(self.CLIP_LAYER,
            self.tr('Clip layer'), [ParameterVector.VECTOR_TYPE_POLYGON], False))
        self.addParameter(ParameterString(self.OPTIONS,
            self.tr('Additional creation options'), '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)[1:-1]
        clipLayer = self.getParameterValue(self.CLIP_LAYER)
        ogrClipLayer = self.ogrConnectionString(clipLayer)

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = self.ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        arguments.append('-clipsrc')
        arguments.append(ogrClipLayer)
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

        GdalUtils.runGdal(commands, progress)
