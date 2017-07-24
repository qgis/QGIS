# -*- coding: utf-8 -*-

"""
***************************************************************************
    DropGeometry.py
    --------------
    Date                 : November 2016
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsWkbTypes,
                       QgsCoordinateReferenceSystem)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DropGeometry(QgisFeatureBasedAlgorithm):

    def tags(self):
        return self.tr('remove,drop,delete,geometry,objects').split(',')

    def group(self):
        return self.tr('Vector general tools')

    def __init__(self):
        super().__init__()

    def name(self):
        return 'dropgeometries'

    def displayName(self):
        return self.tr('Drop geometries')

    def outputName(self):
        return self.tr('Dropped geometries')

    def outputCrs(self, input_crs):
        return QgsCoordinateReferenceSystem()

    def outputWkbType(self, input_wkb_type):
        return QgsWkbTypes.NoGeometry

    def processFeature(self, feature, feedback):
        feature.clearGeometry()
        return feature
