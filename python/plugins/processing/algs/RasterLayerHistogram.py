# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerHistogram.py
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

from processing.outputs.OutputTable import OutputTable
import matplotlib.pyplot as plt
import matplotlib.pylab as lab
from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.outputs.OutputHTML import OutputHTML
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterRaster import ParameterRaster
from processing.tools import raster

class RasterLayerHistogram(GeoAlgorithm):

    INPUT = "INPUT"
    PLOT = "PLOT"
    TABLE = "TABLE"
    BINS = "BINS"


    def processAlgorithm(self, progress):
        uri = self.getParameterValue(self.INPUT)
        layer = QGisLayers.getObjectFromUri(uri)
        outputplot = self.getOutputValue(self.PLOT)
        outputtable = self.getOutputFromName(self.TABLE)
        values = raster.scanraster(layer, progress)
        nbins = self.getParameterValue(self.BINS)
        #ALERT:this is potentially blocking if the layer is too big
        plt.close()
        valueslist = []
        for v in values:
            if v is not None:
                valueslist.append(v)
        n, bins, values = plt.hist(valueslist, nbins)
        fields = [QgsField("CENTER_VALUE", QVariant.Double), QgsField("NUM_ELEM", QVariant.Double)]
        writer = outputtable.getTableWriter(fields)
        for i in xrange(len(values)):
            writer.addRecord([str(bins[i]) + "-" + str(bins[i+1]) , n[i]])
        plotFilename = outputplot +".png"
        lab.savefig(plotFilename)
        f = open(outputplot, "w")
        f.write("<img src=\"" + plotFilename + "\"/>")
        f.close()

    def defineCharacteristics(self):
        self.name = "Raster layer histogram"
        self.group = "Graphics"
        self.addParameter(ParameterRaster(self.INPUT, "Input layer"))
        self.addParameter(ParameterNumber(self.BINS, "Number of bins", 2, None, 10))
        self.addOutput(OutputHTML(self.PLOT, "Output plot"))
        self.addOutput(OutputTable(self.TABLE, "Output table"))

