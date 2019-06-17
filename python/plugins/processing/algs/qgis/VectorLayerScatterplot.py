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

import plotly as plt
import plotly.graph_objs as go

from qgis.core import (QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingUtils,
                       QgsProcessingParameterFileDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class VectorLayerScatterplot(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    XFIELD = 'XFIELD'
    YFIELD = 'YFIELD'

    def group(self):
        return self.tr('Graphics')

    def groupId(self):
        return 'graphics'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.XFIELD,
                                                      self.tr('X attribute'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Numeric))
        self.addParameter(QgsProcessingParameterField(self.YFIELD,
                                                      self.tr('Y attribute'),
                                                      parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Numeric))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT, self.tr('Scatterplot'), self.tr('HTML files (*.html)')))

    def name(self):
        return 'vectorlayerscatterplot'

    def displayName(self):
        return self.tr('Vector layer scatterplot')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        xfieldname = self.parameterAsString(parameters, self.XFIELD, context)
        yfieldname = self.parameterAsString(parameters, self.YFIELD, context)

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        values = vector.values(source, xfieldname, yfieldname)
        data = [go.Scatter(x=values[xfieldname],
                           y=values[yfieldname],
                           mode='markers')]
        plt.offline.plot(data, filename=output, auto_open=False)

        return {self.OUTPUT: output}
