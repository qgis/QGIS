# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
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

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector

from processing.tools import dataobjects, vector

class Merge(GeoAlgorithm):
    LAYER1 = 'LAYER1'
    LAYER2 = 'LAYER2'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Merge vector layers'
        self.group = 'Vector general tools'

        self.addParameter(ParameterVector(self.LAYER1,
            self.tr('Input layer 1'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(self.LAYER2,
            self.tr('Input layer 2'), [ParameterVector.VECTOR_TYPE_ANY]))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Output')))

    def processAlgorithm(self, progress):
        layer1 = dataobjects.getObjectFromUri(
            self.getParameterValue(self.LAYER1))
        layer2 = dataobjects.getObjectFromUri(
            self.getParameterValue(self.LAYER2))

        if layer1.wkbType() != layer2.wkbType():
            raise GeoAlgorithmExecutionException(
                self.tr('Merged layers must have be same type of geometry'))

        count = 0
        fields = []
        layers = [layer1, layer2]
        for layer in layers:
            count += layer.featureCount()

            for sfield in layer.pendingFields():
                found = None
                for dfield in fields:
                    if dfield.name() == sfield.name() and \
                            dfield.type() == sfield.type():
                        found = dfield
                        break

                if not found:
                    fields.append(sfield)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, layer1.wkbType(), layer1.crs())

        total = 100.0 / float(count)
        count = 0
        for layer in layers:
            idx = {}
            for dfield in fields:
                i = 0
                for sfield in layer.pendingFields():
                    if sfield.name() == dfield.name() and \
                            sfield.type() == dfield.type():
                        idx[dfield] = i
                        break
                    i += 1

            features = vector.features(layer)
            for f in features:
                sAttributes = f.attributes()
                dAttributes = []
                for dfield in fields:
                    if dfield in idx:
                        dAttributes.append(sAttributes[idx[dfield]])
                    else:
                        dAttributes.append(dfield.type())
                f.setAttributes(dAttributes)
                writer.addFeature(f)

                count += 1
                progress.setPercentage(int(count * total))

        del writer
