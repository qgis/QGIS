# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomSelectionWithinSubsets.py
    ---------------------
    Date                 : August 2012
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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import random
from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomExtractWithinSubsets(GeoAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    METHODS = ['Number of selected features',
               'Percentage of selected features']

    def defineCharacteristics(self):
        self.name = 'Random extract within subsets'
        self.group = 'Vector selection tools'

        self.addParameter(ParameterVector(self.INPUT, 'Input layer',
                          [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD, 'ID Field',
                          self.INPUT))
        self.addParameter(ParameterSelection(self.METHOD, 'Method',
                          self.METHODS, 0))
        self.addParameter(ParameterNumber(self.NUMBER,
                          'Number/percentage of selected features', 1, None,
                          10))

        self.addOutput(OutputVector(self.OUTPUT, 'Selection'))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)

        layer = dataobjects.getObjectFromUri(filename)
        field = self.getParameterValue(self.FIELD)
        method = self.getParameterValue(self.METHOD)

        index = layer.fieldNameIndex(field)

        features = vector.features(layer)
        featureCount = len(features)
        unique = vector.getUniqueValues(layer, index)
        value = int(self.getParameterValue(self.NUMBER))
        if method == 0:
            if value > featureCount:
                raise GeoAlgorithmExecutionException(
                        'Selected number is greater that feature count. \
                        Choose lesser value and try again.')
        else:
            if value > 100:
                raise GeoAlgorithmExecutionException(
                        "Percentage can't be greater than 100. Set correct \
                        value and try again.")
            value = value / 100.0


        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter(layer.fields(),
                layer.geometryType(), layer.crs())

        selran = []
        current = 0
        total = 100.0 / float(featureCount * len(unique))
        features = vector.features(layer)

        if not len(unique) == featureCount:
            for classValue in unique:
                classFeatures = []
                for i, feature in enumerate(features):
                    attrs = feature.attributes()
                    if attrs[index] == classValue:
                        classFeatures.append(i)
                    current += 1
                    progress.setPercentage(int(current * total))

                if method == 1:
                    selValue = int(round(value * len(classFeatures), 0))
                else:
                    selValue = value

                if selValue >= len(classFeatures):
                    selFeat = classFeatures
                else:
                    selFeat = random.sample(classFeatures, selValue)

                selran.extend(selFeat)
        else:
            selran = range(0, featureCount)


        features = vector.features(layer)
        for (i, feat) in enumerate(features):
            if i in selran:
                writer.addFeature(feat)
            progress.setPercentage(100 * i / float(featureCount))
        del writer
