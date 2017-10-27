# -*- coding: utf-8 -*-

"""
***************************************************************************
    DensifyGeometriesInterval.py by Anita Graser, Dec 2012
    based on DensifyGeometries.py
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

__author__ = 'Anita Graser'
__date__ = 'Dec 2012'
__copyright__ = '(C) 2012, Anita Graser'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingParameterNumber,
                       QgsProcessing)

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DensifyGeometriesInterval(QgisFeatureBasedAlgorithm):

    INTERVAL = 'INTERVAL'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.interval = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.INTERVAL,
                                                       self.tr('Interval between vertices to add'), QgsProcessingParameterNumber.Double,
                                                       1, False, 0, 10000000))

    def name(self):
        return 'densifygeometriesgivenaninterval'

    def displayName(self):
        return self.tr('Densify geometries given an interval')

    def outputName(self):
        return self.tr('Densified')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine, QgsProcessing.TypeVectorPolygon]

    def prepareAlgorithm(self, parameters, context, feedback):
        interval = self.parameterAsDouble(parameters, self.INTERVAL, context)
        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            new_geometry = feature.geometry().densifyByDistance(float(interval))
            feature.setGeometry(new_geometry)
        return feature
