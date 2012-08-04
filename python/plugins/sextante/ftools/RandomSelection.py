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

class RandomSelection(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    METHOD = "METHOD"
    NUMBER = "NUMBER"
    PERCENTAGE = "PERCENTAGE"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/random_selection.png")

    def processAlgorithm(self, progress):
        filename = self.getParameterValue(RandomSelection.INPUT)
        layer = QGisLayers.getObjectFromUri(filename)
        method = self.getParameterValue(self.METHOD)
        if method == 0:
            value = int(self.getParameterValue(self.NUMBER))
        else:
            value = self.getParameterValue(self.PERCENTAGE)
            value = int(round((value / 100.0000), 4) * layer.featureCount())
        selran = random.sample(xrange(0, layer.featureCount()), value)
        layer.setSelectedFeatures(selran)
        self.setOutputValue(self.OUTPUT, filename)


    def defineCharacteristics(self):
        self.name = "Random selection"
        self.group = "Research tools"
        self.addParameter(ParameterVector(RandomSelection.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterSelection(RandomSelection.METHOD, "Method", ["Number of selected features", "Percentage of selected features"]))
        self.addParameter(ParameterNumber(RandomSelection.NUMBER, "Number of selected features", 1, None, 10))
        self.addParameter(ParameterNumber(RandomSelection.PERCENTAGE, "Percentage of selected features", 0, 100, 50))
        self.addOutput(OutputVector(RandomSelection.OUTPUT, "Selection", True))
