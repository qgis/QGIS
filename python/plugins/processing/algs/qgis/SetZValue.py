# -*- coding: utf-8 -*-

"""
***************************************************************************
    SetZValue.py
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


class SetZValue(QgisFeatureBasedAlgorithm):

    Z_VALUE = 'Z_VALUE'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.z_value = 0

    def name(self):
        return 'setzvalue'

    def displayName(self):
        return self.tr('Set Z value')

    def outputName(self):
        return self.tr('Z Added')

    def tags(self):
        return self.tr('set,add,z,25d,3d,values').split(',')

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.Z_VALUE,
                                                       self.tr('Z Value'), QgsProcessingParameterNumber.Double, defaultValue=0.0))

    def outputWkbType(self, inputWkb):
        return QgsWkbTypes.addZ(inputWkb)

    def prepareAlgorithm(self, parameters, context, feedback):
        self.z_value = self.parameterAsDouble(parameters, self.Z_VALUE, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            new_geom = input_geometry.constGet().clone()
            if QgsWkbTypes.hasZ(new_geom.wkbType()):
                # addZValue won't alter existing Z values, so drop them first
                new_geom.dropZValue()

            new_geom.addZValue(self.z_value)

            feature.setGeometry(QgsGeometry(new_geom))

        return feature
