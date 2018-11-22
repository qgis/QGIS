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
                       QgsPropertyDefinition,
                       QgsProcessingParameters,
                       QgsProcessingParameterNumber,
                       QgsProcessingFeatureSource)


from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SetZValue(QgisFeatureBasedAlgorithm):

    Z_VALUE = 'Z_VALUE'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()
        self.z_value = 0
        self.dynamic_z = False
        self.z_property = None

    def name(self):
        return 'setzvalue'

    def displayName(self):
        return self.tr('Set Z value')

    def outputName(self):
        return self.tr('Z Added')

    def tags(self):
        return self.tr('set,add,z,25d,3d,values').split(',')

    def initParameters(self, config=None):
        z_param = QgsProcessingParameterNumber(self.Z_VALUE,
                                               self.tr('Z Value'), QgsProcessingParameterNumber.Double, defaultValue=0.0)
        z_param.setIsDynamic(True)
        z_param.setDynamicLayerParameterName('INPUT')
        z_param.setDynamicPropertyDefinition(QgsPropertyDefinition(self.Z_VALUE, self.tr("Z Value"), QgsPropertyDefinition.Double))
        self.addParameter(z_param)

    def outputWkbType(self, inputWkb):
        return QgsWkbTypes.addZ(inputWkb)

    def sourceFlags(self):
        return QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks

    def prepareAlgorithm(self, parameters, context, feedback):
        self.z_value = self.parameterAsDouble(parameters, self.Z_VALUE, context)
        self.dynamic_z = QgsProcessingParameters.isDynamic(parameters, self.Z_VALUE)
        if self.dynamic_z:
            self.z_property = parameters[self.Z_VALUE]
        return True

    def processFeature(self, feature, context, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            new_geom = input_geometry.constGet().clone()
            if QgsWkbTypes.hasZ(new_geom.wkbType()):
                # addZValue won't alter existing Z values, so drop them first
                new_geom.dropZValue()

            z = self.z_value
            if self.dynamic_z:
                z, ok = self.z_property.valueAsDouble(context.expressionContext(), z)
            new_geom.addZValue(z)

            feature.setGeometry(QgsGeometry(new_geom))

        return [feature]

    def supportInPlaceEdit(self, layer):
        return super().supportInPlaceEdit(layer) and QgsWkbTypes.hasZ(layer.wkbType())
