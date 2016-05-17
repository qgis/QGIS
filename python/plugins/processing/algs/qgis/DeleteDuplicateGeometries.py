# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteDuplicateGeometries.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsGeometry, QgsFeatureRequest
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteDuplicateGeometries(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Delete duplicate geometries')
        self.group, self.i18n_group = self.trAlgorithm('Vector general tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Cleaned')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))

        fields = layer.pendingFields()

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                                                                     layer.wkbType(), layer.crs())

        features = vector.features(layer)

        total = 100.0 / len(features)
        geoms = dict()
        for current, f in enumerate(features):
            geoms[f.id()] = QgsGeometry(f.geometry())
            progress.setPercentage(int(current * total))

        cleaned = dict(geoms)

        for i, g in geoms.iteritems():
            for j in cleaned.keys():
                if i == j or i not in cleaned:
                    continue
                if g.isGeosEqual(cleaned[j]):
                    del cleaned[j]

        total = 100.0 / len(cleaned)
        request = QgsFeatureRequest().setFilterFids(cleaned.keys())
        for current, f in enumerate(layer.getFeatures(request)):
            writer.addFeature(f)
            progress.setPercentage(int(current * total))

        del writer
