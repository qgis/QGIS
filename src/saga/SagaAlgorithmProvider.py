import os
from saga.SagaAlgorithm import SagaAlgorithm
from saga.UnwrappableSagaAlgorithmException import UnwrappableSagaAlgorithmException
from core.AlgorithmProvider import AlgorithmProvider
from saga.SagaUtils import SagaUtils

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
                    self._algs.append(alg)
                except Exception:
                    pass









