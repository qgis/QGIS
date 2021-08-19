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

import warnings

from qgis.core import (QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingException)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import raster

from qgis.PyQt.QtCore import QCoreApplication


class RasterLayerHistogram(QgisAlgorithm):

    INPUT = 'INPUT'
    BINS = 'BINS'
    OUTPUT = 'OUTPUT'
    BAND = 'BAND'

    def group(self):
        return self.tr('Plots')

    def groupId(self):
        return 'plots'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterRasterLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterBand(self.BAND,
                                                     self.tr('Band number'),
                                                     1,
                                                     self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(self.BINS,
                                                       self.tr('number of bins'), minValue=2, defaultValue=10))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT, self.tr('Histogram'), self.tr('HTML files (*.html)')))

    def name(self):
        return 'rasterlayerhistogram'

    def displayName(self):
        return self.tr('Raster layer histogram')

    def processAlgorithm(self, parameters, context, feedback):
        try:
            # importing plotly throws Python warnings from within the library - filter these out
            with warnings.catch_warnings():
                warnings.filterwarnings("ignore", category=ResourceWarning)
                warnings.filterwarnings("ignore", category=ImportWarning)
                import plotly as plt
                import plotly.graph_objs as go
        except ImportError:
            raise QgsProcessingException(QCoreApplication.translate('RasterLayerHistogram', 'This algorithm requires the Python “plotly” library. Please install this library and try again.'))

        layer = self.parameterAsRasterLayer(parameters, self.INPUT, context)
        band = self.parameterAsInt(parameters, self.BAND, context)
        nbins = self.parameterAsInt(parameters, self.BINS, context)

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        # ALERT: this is potentially blocking if the layer is too big
        values = raster.scanraster(layer, feedback, band)

        valueslist = [
            v
            for v in values
            if v is not None
        ]
        data = [go.Histogram(x=valueslist,
                             nbinsx=nbins)]
        plt.offline.plot(data, filename=output, auto_open=False)

        return {self.OUTPUT: output}
