# -*- coding: utf-8 -*-

"""
***************************************************************************
    MeanAndStdDevPlot.py
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

from qgis.core import (QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterFileDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from processing.tools import vector


class MeanAndStdDevPlot(QgisAlgorithm):

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
                                                              self.tr('Input table')))
        self.addParameter(QgsProcessingParameterField(self.NAME_FIELD,
                                                      self.tr('Category name field'), parentLayerParameterName=self.INPUT,
                                                      type=QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterField(self.VALUE_FIELD,
                                                      self.tr('Value field'), parentLayerParameterName=self.INPUT))

        self.addParameter(QgsProcessingParameterFileDestination(self.OUTPUT, self.tr('Plot'), self.tr('HTML files (*.html)')))

    def name(self):
        return 'meanandstandarddeviationplot'

    def displayName(self):
        return self.tr('Mean and standard deviation plot')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        namefieldname = self.parameterAsString(parameters, self.NAME_FIELD, context)
        valuefieldname = self.parameterAsString(parameters, self.VALUE_FIELD, context)

        output = self.parameterAsFileOutput(parameters, self.OUTPUT, context)

        values = vector.values(source, namefieldname, valuefieldname)

        d = {}
        for i in range(len(values[namefieldname])):
            v = values[namefieldname][i]
            if v not in d:
                d[v] = [values[valuefieldname][i]]
            else:
                d[v].append(values[valuefieldname][i])

        data = []
        for k, v in d.items():
            data.append(go.Box(y=list(v),
                               boxmean='sd',
                               name=k
                               ))
        plt.offline.plot(data, filename=output, auto_open=False)

        return {self.OUTPUT: output}
