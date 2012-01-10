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
            providerAlgs = provider.algs
            algs = {}
            for alg in providerAlgs:
                algs[alg.commandLineName()] = alg
            Sextante.algs[provider.getName()] = algs

    @staticmethod
    def getAlgorithm(name):
        for provider in Sextante.algs.values():
            if name in provider:
                return provider[name]
        return None

    @staticmethod
    def asStr():
        s=""
        for alg in Sextante.algs.values():
            s+=(str(alg) + "\n")
        s+=str(len(Sextante.algs)) + " algorithms"
        return s







