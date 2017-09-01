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
from builtins import str

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
from processing.tools import dataobjects
from processing.algs.gdal.GdalUtils import GdalUtils

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class gdaltindex(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD_NAME = 'FIELD_NAME'
    PROJ_DIFFERENCE = 'PROJ_DIFFERENCE'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(ParameterMultipleInput(self.INPUT,
                                                 self.tr('Input layers'), dataobjects.TYPE_RASTER))
        self.addParameter(ParameterString(self.FIELD_NAME,
                                          self.tr('Tile index field'),
                                          'location', optional=True))
        self.addParameter(ParameterBoolean(self.PROJ_DIFFERENCE,
                                           self.tr('Skip files with different projection reference'), False))
        self.addOutput(OutputVector(gdaltindex.OUTPUT, self.tr('Tile index')))

    def name(self):
        return 'tileindex'

    def displayName(self):
        return self.tr('Tile Index')

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'tiles.png'))

    def group(self):
        return self.tr('Raster miscellaneous')

    def getConsoleCommands(self, parameters, context, feedback):
        fieldName = str(self.getParameterValue(self.FIELD_NAME))

        arguments = []
        if len(fieldName) > 0:
            arguments.append('-tileindex')
            arguments.append(fieldName)
        if self.getParameterValue(gdaltindex.PROJ_DIFFERENCE):
            arguments.append('-skip_different_projection')
        arguments.append(str(self.getOutputValue(gdaltindex.OUTPUT)))
        arguments.extend(str(self.getParameterValue(gdaltindex.INPUT)).split(';'))

        return ['gdaltindex', GdalUtils.escapeAndJoin(arguments)]
