# -*- coding: utf-8 -*-

"""
***************************************************************************
    BoundingBox.py
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
                       QgsWkbTypes,
                       QgsProcessingException)


from qgis.PyQt.QtGui import QIcon

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class BoundingBox(QgisFeatureBasedAlgorithm):

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'matrix.png'))

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def name(self):
        return 'boundingboxes'

    def displayName(self):
        return self.tr('Bounding boxes')

    def outputName(self):
        return self.tr('Bounds')

    def outputWkbType(self, inputWkb):
        return QgsWkbTypes.Polygon

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = QgsGeometry.fromRect(input_geometry.boundingBox())
            if not output_geometry:
                raise QgsProcessingException(
                    self.tr('Error calculating bounding box'))

            feature.setGeometry(output_geometry)

        return feature
