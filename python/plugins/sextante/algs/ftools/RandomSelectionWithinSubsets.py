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
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers
from sextante.algs.ftools import FToolsUtils as utils
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterTableField import ParameterTableField

from sextante.outputs.OutputVector import OutputVector

class RandomSelectionWithinSubsets(GeoAlgorithm):

    INPUT = "INPUT"
    METHOD = "METHOD"
    NUMBER = "NUMBER"
    FIELD = "FIELD"
    OUTPUT = "OUTPUT"

    METHODS = ["Number of selected features",
               "Percentage of selected features"
              ]

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/random_selection.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.allowOnlyOpenedLayers = True
        self.name = "Random selection within subsets"
        self.group = "Vector selection tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterTableField(self.FIELD, "ID Field", self.INPUT))
        self.addParameter(ParameterSelection(self.METHOD, "Method", self.METHODS, 0))
        self.addParameter(ParameterNumber(self.NUMBER, "Number/percentage of selected features", 1, None, 10))

        self.addOutput(OutputVector(self.OUTPUT, "Selection", True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)

        layer = QGisLayers.getObjectFromUri(filename)
        field = self.getParameterValue(self.FIELD)
        method = self.getParameterValue(self.METHOD)

        layer.removeSelection()
        index = layer.fieldNameIndex(field)

        unique = utils.getUniqueValues(layer, index)
        featureCount = layer.featureCount()

        value = int(self.getParameterValue(self.NUMBER))
        if method == 0:
            if value > featureCount:
                raise GeoAlgorithmExecutionException("Selected number is greater that feature count. Choose lesser value and try again.")
        else:
            if value > 100:
                raise GeoAlgorithmExecutionException("Persentage can't be greater than 100. Set corrent value and try again.")
            value = value / 100.0

        selran = []
        inFeat = QgsFeature()

        current = 0
        total = 100.0 / float(featureCount * len(unique))

        features = QGisLayers.features(layer)

        if not len(unique) == featureCount:
            for i in unique:
                FIDs= []
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
