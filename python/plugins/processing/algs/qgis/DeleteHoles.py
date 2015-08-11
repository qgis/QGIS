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

from qgis.core import QgsFeature, QgsGeometry
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteHoles(GeoAlgorithm):
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Delete holes')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POLYGON]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Cleaned')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.pendingFields(),
            layer.wkbType(),
            layer.crs())

        features = vector.features(layer)
        count = len(features)
        total = 100.0 / float(count)

        feat = QgsFeature()
        for count, f in enumerate(features):

            geometry = f.geometry()
            if geometry.isMultipart():
                multi_polygon = geometry.asMultiPolygon()
                for polygon in multi_polygon:
                    for ring in polygon[1:]:
                        polygon.remove(ring)
                geometry = QgsGeometry.fromMultiPolygon(multi_polygon)

            else:
                polygon = geometry.asPolygon()
                for ring in polygon[1:]:
                    polygon.remove(ring)
                geometry = QgsGeometry.fromPolygon(polygon)

            feat.setGeometry(geometry)
            feat.setAttributes(f.attributes())
            writer.addFeature(feat)
            progress.setPercentage(int(count * total))

        del writer
