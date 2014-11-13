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

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterExtent
from processing.core.outputs import OutputVector

from processing.tools.system import *

from processing.algs.gdal.OgrAlgorithm import OgrAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

class Ogr2OgrClipExtent(OgrAlgorithm):

    OUTPUT_LAYER = 'OUTPUT_LAYER'
    INPUT_LAYER = 'INPUT_LAYER'
    CLIP_EXTENT = 'CLIP_EXTENT'
    OPTIONS = 'OPTIONS'

    def defineCharacteristics(self):
        self.name = 'Clip vectors by extent'
        self.group = '[OGR] Miscellaneous'

        self.addParameter(ParameterVector(self.INPUT_LAYER, 'Input layer',
                          [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterExtent(self.CLIP_EXTENT,
                          'Clip extent'))
	#self.addParameter(ParameterString(self.CLIP_EXTENT, 'Clip extent',
        #                  '', optional=False))
        self.addParameter(ParameterString(self.OPTIONS, 'Creation Options',
                          '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, 'Output layer'))

    def processAlgorithm(self, progress):
        inLayer = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(inLayer)
        clipExtent = self.getParameterValue(self.CLIP_EXTENT)
        ogrclipExtent = self.ogrConnectionString(clipExtent)

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outFile = output.value

        output = self.ogrConnectionString(outFile)
        options = unicode(self.getParameterValue(self.OPTIONS))

        arguments = []
        regionCoords = ogrclipExtent.split(',')
        arguments.append('-spat')
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[2])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[3])
        #arguments.append('-spat')
        #arguments.append(ogrclipExtent)

        if len(options) > 0:
            arguments.append(options)

        arguments.append(output)
        arguments.append(ogrLayer)

        commands = []
        if isWindows():
            commands = ['cmd.exe', '/C ', 'ogr2ogr.exe',
                        GdalUtils.escapeAndJoin(arguments)]
        else:
            commands = ['ogr2ogr', GdalUtils.escapeAndJoin(arguments)]

        GdalUtils.runGdal(commands, progress)