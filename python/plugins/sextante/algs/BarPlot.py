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
from PyQt4.QtCore import *
from qgis.core import *
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputHTML import OutputHTML
from sextante.tools import *
from sextante.core.QGisLayers import QGisLayers

class BarPlot(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NAME_FIELD = "NAME_FIELD"
    VALUE_FIELD = "VALUE_FIELD"

    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.INPUT)
        layer = QGisLayers.getObjectFromUri(uri)
        namefieldname = self.getParameterValue(self.NAME_FIELD)
        valuefieldname = self.getParameterValue(self.VALUE_FIELD)
        output = self.getOutputValue(self.OUTPUT)
        values = vector.getAttributeValues(layer, namefieldname, valuefieldname)
        plt.close()

        ind = np.arange(len(values[namefieldname]))
        width = 0.8
        plt.bar(ind, values[valuefieldname], width, color='r')

        plt.xticks(ind, values[namefieldname], rotation = 45)
        plotFilename = output +".png"
        lab.savefig(plotFilename)
        f = open(output, "w")
        f.write("<img src=\"" + plotFilename + "\"/>")
        f.close()

    def defineCharacteristics(self):
        self.name = "Bar plot"
        self.group = "Graphics"
        self.addParameter(ParameterTable(self.INPUT, "Input table"))
        self.addParameter(ParameterTableField(self.NAME_FIELD, "Category name field", self.INPUT))
        self.addParameter(ParameterTableField(self.VALUE_FIELD, "Value field", self.INPUT))
        self.addOutput(OutputHTML(self.OUTPUT, "Output"))





