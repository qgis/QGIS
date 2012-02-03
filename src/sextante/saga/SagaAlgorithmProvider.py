import os
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.SagaUtils import SagaUtils
from sextante.core.SextanteUtils import SextanteUtils
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class SagaAlgorithmProvider():

    def __init__(self):
        self.loadAlgorithms()
        self.actions = []
        self.contextMenuActions = []
        self.icon = QIcon(os.path.dirname(__file__) + "/saga.png")

    def loadAlgorithms(self):
        self.algs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.startswith("alg"):
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except Exception:
                    pass
                    #SextanteUtils.addToLog(descriptionFile)

    def getName(self):
        return "SAGA"



