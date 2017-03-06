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
import numpy as np

from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterBoolean
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

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Box plot')
        self.group, self.i18n_group = self.trAlgorithm('Graphics')

        self.addParameter(ParameterTable(self.INPUT, self.tr('Input table')))
        self.addParameter(ParameterTableField(self.NAME_FIELD,
                                              self.tr('Category name field'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.VALUE_FIELD,
                                              self.tr('Value field'),
                                              self.INPUT,
                                              ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterBoolean(self.MSD,
                                           self.tr('Show also standard deviation'), False))
        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Box plot')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        namefieldname = self.getParameterValue(self.NAME_FIELD)
        valuefieldname = self.getParameterValue(self.VALUE_FIELD)

        output = self.getOutputValue(self.OUTPUT)

        values = vector.values(layer, namefieldname, valuefieldname)

        msd = self.getParameterValue(self.MSD)

        if not msd:
            data = [go.Box(
                    x=values[namefieldname],
                    y=values[valuefieldname],
                    boxmean=True)]
        else:
            data = [go.Box(
                    x=values[namefieldname],
                    y=values[valuefieldname],
                    boxmean='sd')]


        plt.offline.plot(data, filename=output, auto_open=False)
