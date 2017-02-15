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
from builtins import str
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import plotly as plt
import plotly.graph_objs as go

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterRaster
from processing.core.outputs import OutputHTML
from processing.tools import dataobjects, raster


class RasterLayerHistogram(GeoAlgorithm):

    INPUT = 'INPUT'
    BINS = 'BINS'
    PLOT = 'PLOT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Raster layer histogram')
        self.group, self.i18n_group = self.trAlgorithm('Graphics')

        self.addParameter(ParameterRaster(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterNumber(self.BINS,
                                          self.tr('Number of bins'), 2, None, 10))

        self.addOutput(OutputHTML(self.PLOT, self.tr('Histogram')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        nbins = self.getParameterValue(self.BINS)

        output = self.getOutputValue(self.PLOT)

        # ALERT: this is potentially blocking if the layer is too big
        values = raster.scanraster(layer, feedback)

        valueslist = []
        for v in values:
            if v is not None:
                valueslist.append(v)

        data = [go.Histogram(x=valueslist,
                             nbinsx=nbins)]
        plt.offline.plot(data, filename=output)
