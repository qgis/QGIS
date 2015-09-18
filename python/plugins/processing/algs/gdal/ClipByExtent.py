# -*- coding: utf-8 -*-

"""
***************************************************************************
    ClipByExtent.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputRaster

from processing.algs.gdal.GdalUtils import GdalUtils


class ClipByExtent(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NO_DATA = 'NO_DATA'
    PROJWIN = 'PROJWIN'
    EXTRA = 'EXTRA'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Clip raster by extent')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Extraction')
        self.addParameter(ParameterRaster(
            self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterString(self.NO_DATA,
                                          self.tr("Nodata value, leave blank to take the nodata value from input"),
                                          ''))
        self.addParameter(ParameterExtent(self.PROJWIN, self.tr('Clipping extent')))
        self.addParameter(ParameterString(self.EXTRA,
                                          self.tr('Additional creation parameters'), '', optional=True))
        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Clipped (extent)')))

    def getConsoleCommands(self):
        out = self.getOutputValue(self.OUTPUT)
        noData = unicode(self.getParameterValue(self.NO_DATA))
        projwin = unicode(self.getParameterValue(self.PROJWIN))
        extra = unicode(self.getParameterValue(self.EXTRA))

        arguments = []
        arguments.append('-of')
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        if len(noData) > 0:
            arguments.append('-a_nodata')
            arguments.append(noData)

        regionCoords = projwin.split(',')
        arguments.append('-projwin')
        arguments.append(regionCoords[0])
        arguments.append(regionCoords[3])
        arguments.append(regionCoords[1])
        arguments.append(regionCoords[2])

        if len(extra) > 0:
            arguments.append(extra)

        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        return ['gdal_translate', GdalUtils.escapeAndJoin(arguments)]

    def commandName(self):
        return "gdal_translate"
