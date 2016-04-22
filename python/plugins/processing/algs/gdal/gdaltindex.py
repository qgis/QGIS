# -*- coding: utf-8 -*-

"""
***************************************************************************
    gdaltindex.py
    ---------------------
    Date                 : February 2015
    Copyright            : (C) 2015 by Pedro Venancio
    Email                : pedrongvenancio at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Pedro Venancio'
__date__ = 'February 2015'
__copyright__ = '(C) 2015, Pedro Venancio'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.outputs import OutputVector
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterString
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class gdaltindex(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD_NAME = 'FIELD_NAME'
    PROJ_DIFFERENCE = 'PROJ_DIFFERENCE'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'tiles.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Tile Index')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Miscellaneous')
        self.addParameter(ParameterMultipleInput(self.INPUT,
                                                 self.tr('Input layers'), ParameterMultipleInput.TYPE_RASTER))
        self.addParameter(ParameterString(self.FIELD_NAME,
                                          self.tr('Tile index field'),
                                          'location', optional=True))
        self.addParameter(ParameterBoolean(self.PROJ_DIFFERENCE,
                                           self.tr('Skip files with different projection reference'), False))
        self.addOutput(OutputVector(gdaltindex.OUTPUT, self.tr('Tile index')))

    def getConsoleCommands(self):
        fieldName = unicode(self.getParameterValue(self.FIELD_NAME))

        arguments = []
        if len(fieldName) > 0:
            arguments.append('-tileindex')
            arguments.append(fieldName)
        if self.getParameterValue(gdaltindex.PROJ_DIFFERENCE):
            arguments.append('-skip_different_projection')
        arguments.append(unicode(self.getOutputValue(gdaltindex.OUTPUT)))
        arguments.extend(unicode(self.getParameterValue(gdaltindex.INPUT)).split(';'))

        return ['gdaltindex', GdalUtils.escapeAndJoin(arguments)]
