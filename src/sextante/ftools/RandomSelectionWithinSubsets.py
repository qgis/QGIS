from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterNumber import ParameterNumber
import random
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.ftools import ftools_utils

class RandomSelectionWithinSubsets(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    METHOD = "METHOD"
    NUMBER = "NUMBER"
    PERCENTAGE = "PERCENTAGE"
    FIELD = "FIELD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/random_selection.png")

    def processAlgorithm(self, progress):
        method = self.getParameterValue(self.METHOD)
        field = self.getParameterValue(RandomSelectionWithinSubsets.FIELD)
        filename = self.getParameterValue(RandomSelectionWithinSubsets.INPUT)
        vlayer = QGisLayers.getObjectFromUri(filename)
        vlayer.removeSelection(True)
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select(allAttrs)
        index = vprovider.fieldNameIndex(field)
        unique = ftools_utils.getUniqueValues(vprovider, int(index))
        inFeat = QgsFeature()
        selran = []
        nFeat = vprovider.featureCount() * len(unique)
        nElement = 0
        if not len(unique) == vlayer.featureCount():
            for i in unique:
                vprovider.rewind()
                FIDs= []
                while vprovider.nextFeature(inFeat):
                    atMap = inFeat.attributeMap()
                    if atMap[index] == QVariant(i):
                        FID = inFeat.id()
                        FIDs.append(FID)
                    nElement += 1
                    progress.setPercentage(nElement/nFeat * 100)
                if method == 0:
                    value = int(self.getParameterValue(self.NUMBER))
                else:
                    value = self.getParameterValue(self.PERCENTAGE)
                    value = int(round((value / 100.0000) * len(FIDs), 0))
                if value >= len(FIDs):
                    selFeat = FIDs
                else:
                    selFeat = random.sample(FIDs, value)
                selran.extend(selFeat)
            vlayer.setSelectedFeatures(selran)
        else:
            vlayer.setSelectedFeatures(range(0, vlayer.featureCount()))
        self.setOutputValue(RandomSelectionWithinSubsets.OUTPUT, filename)


    def defineCharacteristics(self):
        self.name = "Random selection within subsets"
        self.group = "Research tools"
        self.addParameter(ParameterVector(RandomSelectionWithinSubsets.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(RandomSelectionWithinSubsets.FIELD, "ID Field", RandomSelectionWithinSubsets.INPUT))
        self.addParameter(ParameterSelection(RandomSelectionWithinSubsets.METHOD, "Method", ["Number of selected features", "Percentage of selected features"]))
        self.addParameter(ParameterNumber(RandomSelectionWithinSubsets.NUMBER, "Number of selected features", 1, None, 10))
        self.addParameter(ParameterNumber(RandomSelectionWithinSubsets.PERCENTAGE, "Percentage of selected features", 0, 100, 50))
        self.addOutput(OutputVector(RandomSelectionWithinSubsets.OUTPUT, "Selection", True))
