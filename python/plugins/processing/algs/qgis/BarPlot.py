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

import plotly as plt
import plotly.graph_objs as go


from qgis.core import (QgsFeatureRequest,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingException,
                       QgsProcessingParameterFileDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class BarPlot(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    NAME_FIELD = 'NAME_FIELD'
    VALUE_FIELD = 'VALUE_FIELD'

    def group(self):
        return self.tr('Graphics')

    def groupId(self):
        return 'graphics'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.NAME_FIELD,
                                                      self.tr('Category name field'),
                                                      None, self.INPUT, QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterField(self.VALUE_FIELD,
                                                      self.tr('Value field'),
                                                      None, self.INPUT, QgsProcessingParameterField.Numeric))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT, self.tr('Bar plot'), self.tr('HTML files (*.html)')))

    def name(self):
        return 'barplot'

    def displayName(self):
        return self.tr('Bar plot')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        namefieldname = self.parameterAsString(parameters, self.NAME_FIELD, context)
        valuefieldname = self.parameterAsString(parameters, self.VALUE_FIELD, context)

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        values = vector.values(source, valuefieldname)

        x_var = vector.convert_nulls([i[namefieldname] for i in source.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))], '<NULL>')

        data = [go.Bar(x=x_var,
                       y=values[valuefieldname])]
        plt.offline.plot(data, filename=output, auto_open=False)

        return {self.OUTPUT: output}
