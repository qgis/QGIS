# -*- coding: utf-8 -*-

"""
***************************************************************************
    TextToFloat.py
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

from PyQt4.QtCore import QVariant
from qgis.core import QgsField
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class TextToFloat(GeoAlgorithm):
    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Text to float'
        self.group = 'Vector table tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input Layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('Text attribute to convert to float'),
            self.INPUT, ParameterTableField.DATA_TYPE_STRING))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        fieldName = self.getParameterValue(self.FIELD)
        idx = layer.fieldNameIndex(fieldName)

        fields = layer.pendingFields()
        fields[idx] = QgsField(fieldName, QVariant.Double, '', 24, 15)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
            layer.wkbType(), layer.crs())

        features = vector.features(layer)

        count = len(features)
        total = 100.0 / float(count)
        for count, f in enumerate(features):
            value = f[idx]
            try:
                if '%' in value:
                    f[idx] = float(value.replace('%', '')) / 100.0
                else:
                    f[idx] = float(value)
            except:
                f[idx] = None

            writer.addFeature(f)
            progress.setPercentage(int(count * total))

        del writer
