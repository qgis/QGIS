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

import matplotlib.pyplot as plt
import matplotlib.pylab as lab

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputHTML
from processing.tools import vector
from processing.tools import dataobjects


class VectorLayerHistogram(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    BINS = 'BINS'

    def defineCharacteristics(self):
        self.name = 'Vector layer histogram'
        self.group = 'Graphics'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Attribute'), self.INPUT,
            ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterNumber(self.BINS,
            self.tr('number of bins'), 2, None, 10))

        self.addOutput(OutputHTML(self.OUTPUT, self.tr('Histogram')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        fieldname = self.getParameterValue(self.FIELD)
        bins = self.getParameterValue(self.BINS)

        output = self.getOutputValue(self.OUTPUT)

        values = vector.values(layer, fieldname)
        plt.close()
        plt.hist(values[fieldname], bins)
        plotFilename = output + '.png'
        lab.savefig(plotFilename)
        f = open(output, 'w')
        f.write('<html><img src="' + plotFilename + '"/></html>')
        f.close()
