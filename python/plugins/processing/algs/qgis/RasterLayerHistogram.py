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

import matplotlib.pyplot as plt
import matplotlib.pylab as lab

from PyQt4.QtCore import *
from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputHTML
from processing.tools import dataobjects
from processing.tools import raster


class RasterLayerHistogram(GeoAlgorithm):

    INPUT = 'INPUT'
    PLOT = 'PLOT'
    TABLE = 'TABLE'
    BINS = 'BINS'

    def defineCharacteristics(self):
        self.name = 'Raster layer histogram'
        self.group = 'Graphics'

        self.addParameter(ParameterRaster(self.INPUT,
            self.tr('Input layer')))
        self.addParameter(ParameterNumber(self.BINS,
           self.tr('Number of bins'), 2, None, 10))

        self.addOutput(OutputHTML(self.PLOT, self.tr('Output plot')))
        self.addOutput(OutputTable(self.TABLE, self.tr('Output table')))


    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        nbins = self.getParameterValue(self.BINS)

        outputplot = self.getOutputValue(self.PLOT)
        outputtable = self.getOutputFromName(self.TABLE)

        values = raster.scanraster(layer, progress)

        # ALERT: this is potentially blocking if the layer is too big
        plt.close()
        valueslist = []
        for v in values:
            if v is not None:
                valueslist.append(v)
        (n, bins, values) = plt.hist(valueslist, nbins)

        fields = [QgsField('CENTER_VALUE', QVariant.Double),
                  QgsField('NUM_ELEM', QVariant.Double)]
        writer = outputtable.getTableWriter(fields)
        for i in xrange(len(values)):
            writer.addRecord([str(bins[i]) + '-' + str(bins[i + 1]), n[i]])

        plotFilename = outputplot + '.png'
        lab.savefig(plotFilename)
        f = open(outputplot, 'w')
        f.write('<img src="' + plotFilename + '"/>')
        f.close()
