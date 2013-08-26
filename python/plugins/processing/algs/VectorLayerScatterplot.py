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
from processing.parameters.ParameterNumber import ParameterNumber

__author__ = 'Victor Olaya'
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import matplotlib.pyplot as plt
import matplotlib.pylab as lab
from PyQt4.QtCore import *
from qgis.core import *
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.outputs.OutputHTML import OutputHTML
from processing.tools import *
from processing.core.QGisLayers import QGisLayers

class VectorLayerScatterplot(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    XFIELD = "XFIELD"
    YFIELD = "YFIELD"


    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.INPUT)
        layer = QGisLayers.getObjectFromUri(uri)
        xfieldname = self.getParameterValue(self.YFIELD)
        yfieldname = self.getParameterValue(self.XFIELD)
        output = self.getOutputValue(self.OUTPUT)
        values = vector.getAttributeValues(layer, xfieldname, yfieldname)
        plt.close()

        plt.scatter(values[xfieldname], values[yfieldname])
        plotFilename = output +".png"
        lab.savefig(plotFilename)
        f = open(output, "w")
        f.write("<img src=\"" + plotFilename + "\"/>")
        f.close()

    def defineCharacteristics(self):
        self.name = "Vector layer scatterplot"
        self.group = "Graphics"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.XFIELD, "X attribute", self.INPUT,ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.YFIELD, "Y attribute", self.INPUT,ParameterTableField.DATA_TYPE_NUMBER))
        self.addOutput(OutputHTML(self.OUTPUT, "Output"))

