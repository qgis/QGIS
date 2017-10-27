# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensifyGeometries.py
    ---------------------
    Date                 : October 2012
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
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'October 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsProcessingParameterNumber,
                       QgsProcessing)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DensifyGeometries(QgisFeatureBasedAlgorithm):

    VERTICES = 'VERTICES'

    def tags(self):
        return self.tr('add,vertices,points').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.vertices = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.VERTICES,
                                                       self.tr('Vertices to add'), QgsProcessingParameterNumber.Integer,
                                                       1, False, 1, 10000000))

    def name(self):
        return 'densifygeometries'

    def displayName(self):
        return self.tr('Densify geometries')

    def outputName(self):
        return self.tr('Densified')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine, QgsProcessing.TypeVectorPolygon]

    def prepareAlgorithm(self, parameters, context, feedback):
        self.vertices = self.parameterAsInt(parameters, self.VERTICES, context)
        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            new_geometry = feature.geometry().densifyByCount(self.vertices)
            feature.setGeometry(new_geometry)
        return feature
