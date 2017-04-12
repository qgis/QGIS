# -*- coding: utf-8 -*-

"""
***************************************************************************
    BarPlot.py
    ---------------------
    Date                 : March 2015
    Copyright            : (C) 2017 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matteo Ghetta'
__date__ = 'March 2017'
__copyright__ = '(C) 2017, Matteo Ghetta'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import plotly as plt
import plotly.graph_objs as go

from qgis.core import (QgsApplication)
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterSelection
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.outputs import OutputHTML
from processing.tools import vector
from processing.tools import dataobjects


class BoxPlot(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NAME_FIELD = 'NAME_FIELD'
    VALUE_FIELD = 'VALUE_FIELD'
    MSD = 'MSD'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Graphics')

    def name(self):
        return 'boxplot'

    def displayName(self):
        return self.tr('Box plot')

    def defineCharacteristics(self):
        self.addParameter(ParameterTable(self.INPUT, self.tr('Input table')))
        self.addParameter(ParameterTableField(self.NAME_FIELD,
                                              self.tr('Category name field'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterTableField(self.VALUE_FIELD,
                                              self.tr('Value field'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        msd = [self.tr('Show Mean'),
               self.tr('Show Standard Deviation'),
               self.tr('Don\'t show Mean and Standard Deviation')
               ]
        self.addParameter(ParameterSelection(
            self.MSD,
            self.tr('Additional Statistic Lines'),
            msd, default=0))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Box plot')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        namefieldname = self.getParameterValue(self.NAME_FIELD)
        valuefieldname = self.getParameterValue(self.VALUE_FIELD)

        output = self.getOutputValue(self.OUTPUT)

        values = vector.values(layer, valuefieldname)

        x_var = [i[namefieldname] for i in layer.getFeatures()]

        msdIndex = self.getParameterValue(self.MSD)
        msd = True

        if msdIndex == 1:
            msd = 'sd'
        elif msdIndex == 2:
            msd = False

        data = [go.Box(
                x=x_var,
                y=values[valuefieldname],
                boxmean=msd)]

        plt.offline.plot(data, filename=output, auto_open=False)
