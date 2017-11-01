# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteHoles.py
    ---------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Etienne Trimaille
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Etienne Trimaille'
__date__ = 'April 2015'
__copyright__ = '(C) 2015, Etienne Trimaille'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsProcessingParameterNumber,
                       QgsProcessing)
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class DeleteHoles(QgisFeatureBasedAlgorithm):

    MIN_AREA = 'MIN_AREA'

    def __init__(self):
        super().__init__()
        self.min_area = None

    def initParameters(self, config=None):
        self.addParameter(QgsProcessingParameterNumber(self.MIN_AREA,
                                                       self.tr('Remove holes with area less than'), QgsProcessingParameterNumber.Double,
                                                       0, True, 0.0, 10000000.0))

    def tags(self):
        return self.tr('remove,delete,drop,holes,rings,fill').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def name(self):
        return 'deleteholes'

    def displayName(self):
        return self.tr('Delete holes')

    def outputName(self):
        return self.tr('Cleaned')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorPolygon]

    def prepareAlgorithm(self, parameters, context, feedback):
        self.min_area = self.parameterAsDouble(parameters, self.MIN_AREA, context)
        if self.min_area == 0.0:
            self.min_area = -1.0
        return True

    def processFeature(self, feature, feedback):
        if feature.hasGeometry():
            feature.setGeometry(feature.geometry().removeInteriorRings(self.min_area))
        return feature
