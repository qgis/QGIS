from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.modeler.CalculatorModelerAlgorithm import \
    CalculatorModelerAlgorithm

import os.path

class ModelerOnlyAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)

    def getName(self):
        return "modelertools"

    def getDescription(self):
        return "Modeler-only tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")

    def _loadAlgorithms(self):
        self.algs = [CalculatorModelerAlgorithm()]
        for alg in self.algs:
            alg.provider = self