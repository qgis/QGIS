from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextanteexampleprovider.ExampleAlgorithm import ExampleAlgorithm

class ExampleAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [ExampleAlgorithm()]
        for alg in self.alglist:
            alg.provider = self

    def getName(self):
        '''This is the name that will appear on the toolbox group.
        It is also used to create the command line name of all the algorithms
        from this provider'''
        return "Example algorithms"

    def getIcon(self):
        '''We return the default icon'''
        return AlgorithmProvider.getIcon(self)


    def _loadAlgorithms(self):
        '''Here we fill the list of algorithms in self.algs.
        This method is called whenever the list of algorithms should be updated.
        If the list of algorithms can change while executing SEXTANTE for QGIS
        (for instance, if it contains algorithms from user-defined scripts and
        a new algorithm might have been added), you should create the list again
        here.
        In this case, since the list is always de same, we assign from the premade list.
        This assignment has to be done in this method even if the list does not change,
        since the self.algs list is cleared before calling this method'''
        self.algs = self.alglist
