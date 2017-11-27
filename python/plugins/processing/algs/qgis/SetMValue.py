# -*- coding: utf-8 -*-

"""
***************************************************************************
    SetMValue.py
    --------------
    Date                 : July 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'July 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessingParameterNumber)


from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SetMValue(QgisFeatureBasedAlgorithm):

    M_VALUE = 'M_VALUE'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.m_value = 0

    def name(self):
        return 'setmvalue'

    def displayName(self):
        return self.tr('Set M value')

    def outputName(self):
        return self.tr('M Added')

    def tags(self):
        return self.tr('set,add,m,measure,values').split(',')

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.M_VALUE,
                                                       self.tr('M Value'), QgsProcessingParameterNumber.Double, defaultValue=0.0))

    def outputWkbType(self, inputWkb):
        return QgsWkbTypes.addM(inputWkb)

    def prepareAlgorithm(self, parameters, context, feedback):
        self.m_value = self.parameterAsDouble(parameters, self.M_VALUE, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            new_geom = input_geometry.constGet().clone()
            if QgsWkbTypes.hasM(new_geom.wkbType()):
                # addMValue won't alter existing M values, so drop them first
                new_geom.dropMValue()

            new_geom.addMValue(self.m_value)

            feature.setGeometry(QgsGeometry(new_geom))

        return feature
