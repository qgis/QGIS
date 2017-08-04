# -*- coding: utf-8 -*-

"""
***************************************************************************
    Boundary.py
    --------------
    Date                 : July 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
__date__ = 'July 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsGeometry,
                       QgsWkbTypes)

from qgis.PyQt.QtGui import QIcon

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Boundary(QgisFeatureBasedAlgorithm):

    def __init__(self):
        super().__init__()

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'convex_hull.png'))

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'boundary'

    def displayName(self):
        return self.tr('Boundary')

    def outputName(self):
        return self.tr('Boundary')

    def outputWkbType(self, input_wkb):
        if QgsWkbTypes.geometryType(input_wkb) == QgsWkbTypes.LineGeometry:
            output_wkb = QgsWkbTypes.MultiPoint
        elif QgsWkbTypes.geometryType(input_wkb) == QgsWkbTypes.PolygonGeometry:
            output_wkb = QgsWkbTypes.MultiLineString
        if QgsWkbTypes.hasZ(input_wkb):
            output_wkb = QgsWkbTypes.addZ(output_wkb)
        if QgsWkbTypes.hasM(input_wkb):
            output_wkb = QgsWkbTypes.addM(output_wkb)

        return output_wkb

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = QgsGeometry(input_geometry.geometry().boundary())
            if not output_geometry:
                feedback.reportError(self.tr('No boundary for feature {} (possibly a closed linestring?)').format(feature.id()))
                feature.clearGeometry()
            else:
                feature.setGeometry(output_geometry)
        return feature
