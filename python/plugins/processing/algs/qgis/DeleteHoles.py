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

from qgis.core import (QgsApplication,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import (ParameterVector,
                                        ParameterNumber)
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteHoles(GeoAlgorithm):

    INPUT = 'INPUT'
    MIN_AREA = 'MIN_AREA'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('remove,delete,drop,holes,rings,fill').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def name(self):
        return 'deleteholes'

    def displayName(self):
        return self.tr('Delete holes')

    def defineCharacteristics(self):
        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [dataobjects.TYPE_VECTOR_POLYGON]))
        self.addParameter(ParameterNumber(self.MIN_AREA,
                                          self.tr('Remove holes with area less than'), 0, 10000000.0, default=0.0, optional=True))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Cleaned'), datatype=[dataobjects.TYPE_VECTOR_POLYGON]))

    def processAlgorithm(self, context, feedback):
        layer = dataobjects.getLayerFromString(
            self.getParameterValue(self.INPUT))
        min_area = self.getParameterValue(self.MIN_AREA)
        if min_area is not None:
            try:
                min_area = float(min_area)
            except:
                pass
        if min_area == 0.0:
            min_area = -1.0

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields(),
            layer.wkbType(),
            layer.crs())

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)

        for current, f in enumerate(features):
            if f.hasGeometry():
                if min_area is not None:
                    f.setGeometry(f.geometry().removeInteriorRings(min_area))
                else:
                    f.setGeometry(f.geometry().removeInteriorRings())
            writer.addFeature(f)
            feedback.setProgress(int(current * total))

        del writer
