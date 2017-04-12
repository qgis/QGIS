# -*- coding: utf-8 -*-

"""
***************************************************************************
    EquivalentNumField.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import plotly as plt
import plotly.graph_objs as go

from qgis.core import (QgsApplication)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputHTML

from processing.tools import vector
from processing.tools import dataobjects


class VectorLayerScatterplot3D(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    XFIELD = 'XFIELD'
    YFIELD = 'YFIELD'
    ZFIELD = 'ZFIELD'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Graphics')

    def name(self):
        return 'scatter3dplot'

    def displayName(self):
        return self.tr('Vector layer scatterplot 3D')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.XFIELD,
                                              self.tr('X attribute'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.YFIELD,
                                              self.tr('Y attribute'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.ZFIELD,
                                              self.tr('Z attribute'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Scatterplot 3D')))

    def processAlgorithm(self, feedback):

        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        xfieldname = self.getParameterValue(self.XFIELD)
        yfieldname = self.getParameterValue(self.YFIELD)
        zfieldname = self.getParameterValue(self.ZFIELD)

        output = self.getOutputValue(self.OUTPUT)

        values = vector.values(layer, xfieldname, yfieldname, zfieldname)

        data = [go.Scatter3d(
                x=values[xfieldname],
                y=values[yfieldname],
                z=values[zfieldname],
                mode='markers')]

        plt.offline.plot(data, filename=output, auto_open=False)
