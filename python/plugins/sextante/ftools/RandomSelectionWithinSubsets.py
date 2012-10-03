import os.path
import random

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers

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

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/random_selection.png")

    def defineCharacteristics(self):
        self.name = "Random selection within subsets"
        self.group = "Research tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.FIELD, "ID Field", self.INPUT))
        self.addParameter(ParameterSelection(self.METHOD, "Method", self.METHODS, 0))
        self.addParameter(ParameterNumber(self.NUMBER, "Number/persentage of selected features", 1, None, 10))

        self.addOutput(OutputVector(self.OUTPUT, "Selection", True))

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(self.INPUT)

        layer = QGisLayers.getObjectFromUri(filename)
        field = self.getParameterValue(self.FIELD)
        method = self.getParameterValue(self.METHOD)

        layer.removeSelection(True)
        provider = layer.dataProvider()
        index = layer.fieldNameIndex(field)
        layer.select([index])

        unique = layer.uniqueValues(index)
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

        if not len(unique) == featureCount:
            for i in unique:
                FIDs= []
                layer.select([index])
                while layer.nextFeature(inFeat):
                    atMap = inFeat.attributeMap()
                    if atMap[index] == QVariant(i):
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
