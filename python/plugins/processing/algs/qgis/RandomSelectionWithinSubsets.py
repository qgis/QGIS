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
from qgis.core import QgsFeature
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomSelectionWithinSubsets(GeoAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    METHODS = ['Number of selected features',
               'Percentage of selected features']

    def defineCharacteristics(self):
        self.allowOnlyOpenedLayers = True
        self.name = 'Random selection within subsets'
        self.group = 'Vector selection tools'

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD,
            self.tr('ID Field'), self.INPUT))
        self.addParameter(ParameterSelection(self.METHOD,
            self.tr('Method'), self.METHODS, 0))
        self.addParameter(ParameterNumber(self.NUMBER,
            self.tr('Number/percentage of selected features'), 1, None, 10))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Selection'), True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)

        layer = dataobjects.getObjectFromUri(filename)
        field = self.getParameterValue(self.FIELD)
        method = self.getParameterValue(self.METHOD)

        layer.removeSelection()
        index = layer.fieldNameIndex(field)

        unique = vector.getUniqueValues(layer, index)
        featureCount = layer.featureCount()

        value = int(self.getParameterValue(self.NUMBER))
        if method == 0:
            if value > featureCount:
                raise GeoAlgorithmExecutionException(
                    self.tr('Selected number is greater that feature count. '
                            'Choose lesser value and try again.'))
        else:
            if value > 100:
                raise GeoAlgorithmExecutionException(
                    self.tr("Percentage can't be greater than 100. Set a "
                            "different value and try again."))
            value = value / 100.0

        selran = []
        inFeat = QgsFeature()

        current = 0
        total = 100.0 / float(featureCount * len(unique))

        if not len(unique) == featureCount:
            for i in unique:
                features = vector.features(layer)
                FIDs = []
                for inFeat in features:
                    attrs = inFeat.attributes()
                    if attrs[index] == i:
                        FIDs.append(inFeat.id())
                    current += 1
                    progress.setPercentage(int(current * total))

                if method == 1:
                    selValue = int(round(value * len(FIDs), 0))
                else:
                    selValue = value

                if selValue >= len(FIDs):
                    selFeat = FIDs
                else:
                    selFeat = random.sample(FIDs, selValue)

                selran.extend(selFeat)
            layer.setSelectedFeatures(selran)
        else:
            layer.setSelectedFeatures(range(0, featureCount))

        self.setOutputValue(self.OUTPUT, filename)
