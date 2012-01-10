from sextante.saga.SagaAlgorithmProvider import SagaAlgorithmProvider

class Sextante:

    providers = [SagaAlgorithmProvider()]
    algs = {}


    def __init__(self):
        pass

    @staticmethod
    def initialize():
        Sextante.loadAlgorithms()

    @staticmethod
    def loadAlgorithms():
        for provider in Sextante.providers:
            algs = provider.algs
            for alg in algs:
                Sextante.algs[alg.commandLineName()] = alg

    @staticmethod
    def getAlgorithm(name):
        return Sextante.algs[name]


    @staticmethod
    def asStr():
        s=""
        for alg in Sextante.algs.values():
            s+=(str(alg) + "\n")
        s+=str(len(Sextante.algs)) + " algorithms"
        return s







