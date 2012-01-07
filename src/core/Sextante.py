from saga.SagaAlgorithmProvider import SagaAlgorithmProvider
import os

class Sextante:

    def __init__(self):
        self._providers = [SagaAlgorithmProvider()]
        self._algs = {}
        self.loadAlgorithms()


    def loadAlgorithms(self):
        for provider in self._providers:
            algs = provider.algs
            for alg in algs:
                self._algs[alg.name] = alg

    def getAlgorithm(self, name):
        return self._algs[name]

    @property
    def algs(self):
        return self._algs

    def __str__(self):
        s=""
        for alg in self.algs.values():
            s+=(str(alg) + "\n")
        s+=str(len(self.algs)) + " algorithms"
        return s


    @staticmethod
    def isWindows():
        return True


    @staticmethod
    def userFolder():
        userfolder = os.getenv('HOME') + os.sep + "sextante"
        mkdir(userfolder)

        return userfolder



def mkdir(newdir):
    if os.path.isdir(newdir):
        pass
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)







