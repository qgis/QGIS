# -*- coding: utf-8 -*-

"""
***************************************************************************
    contour.py
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
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString

from processing.core.outputs import OutputVector

from processing.algs.gdal.GdalUtils import GdalUtils


class contour(GdalAlgorithm):

    INPUT_RASTER = 'INPUT_RASTER'
    OUTPUT_VECTOR = 'OUTPUT_VECTOR'
    INTERVAL = 'INTERVAL'
    FIELD_NAME = 'FIELD_NAME'
    EXTRA = 'EXTRA'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Contour')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Extraction')
        self.addParameter(ParameterRaster(self.INPUT_RASTER,
            self.tr('Input layer'), False))
        self.addParameter(ParameterNumber(self.INTERVAL,
            self.tr('Interval between contour lines'), 0.0,
            99999999.999999, 10.0))
        self.addParameter(ParameterString(self.FIELD_NAME,
            self.tr('Attribute name (if not set, no elevation attribute is attached)'),
            'ELEV', optional=True))
        self.addParameter(ParameterString(self.EXTRA,
            self.tr('Additional creation parameters'), '', optional=True))

        self.addOutput(OutputVector(self.OUTPUT_VECTOR,
            self.tr('Contours')))

    def getConsoleCommands(self):
        interval = str(self.getParameterValue(self.INTERVAL))
        fieldName = str(self.getParameterValue(self.FIELD_NAME))
        extra = str(self.getParameterValue(self.EXTRA))

        arguments = []
        if len(fieldName) > 0:
            arguments.append('-a')
            arguments.append(fieldName)
        arguments.append('-i')
        arguments.append(interval)

        if len(extra) > 0:
            arguments.append(extra)

        arguments.append(self.getParameterValue(self.INPUT_RASTER))
        arguments.append(self.getOutputValue(self.OUTPUT_VECTOR))

        return ['gdal_contour', GdalUtils.escapeAndJoin(arguments)]
