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
from PyQt4.QtCore import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputHTML import OutputHTML
from sextante.tools import *
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterNumber import ParameterNumber

class VectorLayerHistogram(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    BINS = "BINS"


    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.INPUT)
        layer = QGisLayers.getObjectFromUri(uri)
        fieldname = self.getParameterValue(self.FIELD)
        output = self.getOutputValue(self.OUTPUT)
        values = vector.getAttributeValues(layer, fieldname)
        plt.close()
        bins = self.getParameterValue(self.BINS)
        plt.hist(values[fieldname], bins)
        plotFilename = output +".png"
        lab.savefig(plotFilename)
        f = open(output, "w")
        f.write("<img src=\"" + plotFilename + "\"/>")
        f.close()

    def defineCharacteristics(self):
        self.name = "Vector layer histogram"
        self.group = "Graphics"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD, "Attribute", self.INPUT,ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterNumber(self.BINS, "number of bins", 2, None, 10))
        self.addOutput(OutputHTML(self.OUTPUT, "Output"))

