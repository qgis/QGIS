# -*- coding: utf-8 -*-

"""
***************************************************************************
    Smooth.py
    ---------
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

from qgis.core import (QgsProcessing,
                       QgsWkbTypes)

from qgis.PyQt.QtGui import QIcon

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MergeLines(QgisFeatureBasedAlgorithm):

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'to_lines.png'))

    def tags(self):
        return self.tr('line,merge,join,parts').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def name(self):
        return 'mergelines'

    def displayName(self):
        return self.tr('Merge lines')

    def outputName(self):
        return self.tr('Merged')

    def outputType(self):
        return QgsProcessing.TypeVectorLine

    def outputWkbType(self, input_wkb):
        return QgsWkbTypes.MultiLineString

    def processFeature(self, feature, feedback):
        input_geometry = feature.geometry()
        if input_geometry:
            output_geometry = input_geometry.mergeLines()
            if not output_geometry:
                feedback.reportError(self.tr('Error merging lines for feature {}').format(feature.id()))

            feature.setGeometry(output_geometry)
        return feature
