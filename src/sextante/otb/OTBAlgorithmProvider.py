import os
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider

class OTBAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.createAlgsList()

    def getName(self):
        return "OTB"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/icons/otb.png")

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        self.preloadedAlgs = []

