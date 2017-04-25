# -*- coding: utf-8 -*-

"""
***************************************************************************
    TruncateTable.py
    -----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.outputs import OutputVector
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects


class TruncateTable(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('empty,delete,layer,clear,features').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def name(self):
        return 'truncatetable'

    def displayName(self):
        return self.tr('Truncate table')

    def defineCharacteristics(self):
        self.addParameter(ParameterTable(self.INPUT,
                                         self.tr('Input Layer')))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Truncated layer'), True))

    def processAlgorithm(self, context, feedback):
        file_name = self.getParameterValue(self.INPUT)
        layer = dataobjects.getLayerFromString(file_name)
        provider = layer.dataProvider()

        if not provider.truncate():
            raise GeoAlgorithmExecutionException(
                self.tr('Could not truncate table.'))

        self.setOutputValue(self.OUTPUT, file_name)
