# -*- coding: utf-8 -*-

"""
***************************************************************************
    DropMZValues.py
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
                       QgsProcessingParameterBoolean)


from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class DropMZValues(QgisFeatureBasedAlgorithm):

    DROP_M_VALUES = 'DROP_M_VALUES'
    DROP_Z_VALUES = 'DROP_Z_VALUES'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.drop_m = False
        self.drop_z = False

    def name(self):
        return 'dropmzvalues'

    def displayName(self):
        return self.tr('Drop M/Z values')

    def outputName(self):
        return self.tr('Z/M Dropped')

    def tags(self):
        return self.tr('drop,set,convert,m,measure,z,25d,3d,values').split(',')

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterBoolean(self.DROP_M_VALUES,
                                                        self.tr('Drop M Values'), defaultValue=False))
        self.addParameter(QgsProcessingParameterBoolean(self.DROP_Z_VALUES,
                                                        self.tr('Drop Z Values'), defaultValue=False))

    def outputWkbType(self, inputWkb):
        wkb = inputWkb
        if self.drop_m:
            wkb = QgsWkbTypes.dropM(wkb)
        if self.drop_z:
            wkb = QgsWkbTypes.dropZ(wkb)
        return wkb

    def prepareAlgorithm(self, parameters, context, feedback):
        self.drop_m = self.parameterAsBool(parameters, self.DROP_M_VALUES, context)
        self.drop_z = self.parameterAsBool(parameters, self.DROP_Z_VALUES, context)
        return True

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            new_geom = input_geometry.geometry().clone()
            if self.drop_m:
                new_geom.dropMValue()
            if self.drop_z:
                new_geom.dropZValue()
            feature.setGeometry(QgsGeometry(new_geom))

        return feature
