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
    def algList():
        s=""
        for provider in Sextante.algs.values():
            for alg in provider.values():
                s+=(alg.name + " --->" + alg.commandLineName() + "\n")
        print s

    @staticmethod
    def algHelp(name):
        alg = Sextante.getAlgorithm(name)
        if alg != None:
            print(str(alg))
        else:
            print "Algorithm not found"





