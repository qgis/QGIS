# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteColumn.py
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

from qgis.core import QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class DeleteColumn(GeoAlgorithm):
    INPUT = 'INPUT'
    COLUMN = 'COLUMN'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Delete column'
        self.group = 'Vector table tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.COLUMN,
            self.tr('Field to delete'), self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))
        idx = layer.fieldNameIndex(self.getParameterValue(self.COLUMN))

        fields = layer.pendingFields()
        fields.remove(idx)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
            layer.wkbType(), layer.crs())

        features = vector.features(layer)
        count = len(features)
        total = 100.0 / float(count)

        feat = QgsFeature()
        for count, f in enumerate(features):
            feat.setGeometry(f.geometry())
            attributes = f.attributes()
            del attributes[idx]
            feat.setAttributes(attributes)
            writer.addFeature(feat)

            progress.setPercentage(int(count * total))

        del writer
