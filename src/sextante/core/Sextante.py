from sextante.saga.SagaAlgorithmProvider import SagaAlgorithmProvider
from sextante.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider

class Sextante:

    providers = [SagaAlgorithmProvider(), ScriptAlgorithmProvider()]
    algs = {}
    actions = {}
    contextMenuActions = []


    def __init__(self):
        pass

    @staticmethod
    def initialize():
        Sextante.loadAlgorithms()
        Sextante.loadActions()
        Sextante.loadContextMenuActions()

    @staticmethod
    def loadAlgorithms():
        for provider in Sextante.providers:
            providerAlgs = provider.algs
            algs = {}
            for alg in providerAlgs:
                alg.icon = provider.icon
                algs[alg.commandLineName()] = alg
            Sextante.algs[provider.getName()] = algs

    @staticmethod
    def loadActions():
        for provider in Sextante.providers:
            providerActions = provider.actions
            actions = list()
            for action in providerActions:
                action.icon = provider.icon
                actions.append(action)
            Sextante.actions[provider.getName()] = actions

    @staticmethod
    def loadContextMenuActions():
        for provider in Sextante.providers:
            providerActions = provider.contextMenuActions
            for action in providerActions:
                Sextante.contextMenuActions.append(action)

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





