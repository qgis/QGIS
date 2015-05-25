# -*- coding: utf-8 -*-

"""
***************************************************************************
    BarPlot.py
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

import matplotlib.pyplot as plt
import matplotlib.pylab as lab
import numpy as np

from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.outputs import OutputHTML
from processing.tools import vector
from processing.tools import dataobjects


class BarPlot(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NAME_FIELD = 'NAME_FIELD'
    VALUE_FIELD = 'VALUE_FIELD'

    def defineCharacteristics(self):
        self.name = 'Bar plot'
        self.group = 'Graphics'

        self.addParameter(ParameterTable(self.INPUT, self.tr('Input table')))
        self.addParameter(ParameterTableField(self.NAME_FIELD,
            self.tr('Category name field'), self.INPUT))
        self.addParameter(ParameterTableField(self.VALUE_FIELD,
            self.tr('Value field'), self.INPUT))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Bar plot')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        namefieldname = self.getParameterValue(self.NAME_FIELD)
        valuefieldname = self.getParameterValue(self.VALUE_FIELD)

        output = self.getOutputValue(self.OUTPUT)

        values = vector.values(layer, namefieldname, valuefieldname)
        plt.close()

        ind = np.arange(len(values[namefieldname]))
        width = 0.8
        plt.bar(ind, values[valuefieldname], width, color='r')
        plt.xticks(ind, values[namefieldname], rotation=45)
        plotFilename = output + '.png'
        lab.savefig(plotFilename)
        f = open(output, 'w')
        f.write('<html><img src="' + plotFilename + '"/></html>')
        f.close()
