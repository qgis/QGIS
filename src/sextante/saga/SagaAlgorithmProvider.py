import os
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.UnwrappableSagaAlgorithmException import UnwrappableSagaAlgorithmException
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.saga.SagaUtils import SagaUtils

class SagaAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        self._algs = []
        self.loadAlgorithms()

    def loadAlgorithms(self):
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.startswith("alg"):
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self._algs.append(alg)
                except Exception:
                    print (descriptionFile)

    def getName(self):
        return "SAGA"



