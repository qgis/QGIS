# -*- coding: utf-8 -*-

"""
***************************************************************************
    SimplifyGeometries.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsMapToPixelSimplifier,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SimplifyGeometries(QgisFeatureBasedAlgorithm):

    TOLERANCE = 'TOLERANCE'
    METHOD = 'METHOD'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'simplify.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()
        self.tolerance = None
        self.method = None
        self.simplifier = None

    def initParameters(self, config=None):
        self.methods = [self.tr('Distance (Douglas-Peucker)'),
                        'Snap to grid',
                        'Area (Visvalingam)']
        self.addParameter(QgsProcessingParameterEnum(
            self.METHOD,
            self.tr('Simplification method'),
            self.methods, defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.TOLERANCE,
                                                       self.tr('Tolerance'), minValue=0.0, maxValue=10000000.0, defaultValue=1.0))

    def name(self):
        return 'simplifygeometries'

    def displayName(self):
        return self.tr('Simplify geometries')

    def outputName(self):
        return self.tr('Simplified')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.tolerance = self.parameterAsDouble(parameters, self.TOLERANCE, context)
        self.method = self.parameterAsEnum(parameters, self.METHOD, context)
        if self.method != 0:
            self.simplifier = QgsMapToPixelSimplifier(QgsMapToPixelSimplifier.SimplifyGeometry, self.tolerance, self.method)

        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            input_geometry = feature.geometry()

            if self.method == 0:  # distance
                output_geometry = input_geometry.simplify(self.tolerance)
            else:
                output_geometry = self.simplifier.simplify(input_geometry)

            feature.setGeometry(output_geometry)
        return feature
