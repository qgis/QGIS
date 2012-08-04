from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextanteexampleprovider.ExampleAlgorithm import ExampleAlgorithm
from sextante.core.SextanteConfig import Setting, SextanteConfig

class ExampleAlgorithmProvider(AlgorithmProvider):

    MY_DUMMY_SETTING = "MY_DUMMY_SETTING"

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [ExampleAlgorithm()]

    def initializeSettings(self):
        '''In this method we add settings needed to configure our provider.
        Do not forget to call the parent method, since it takes care or
        automatically adding a setting for activating or deactivating the
        algorithms in the provider'''
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting("Example algorithms", ExampleAlgorithmProvider.MY_DUMMY_SETTING, "Example setting", "Default value"))
        '''To get the parameter of a setting parameter, use SextanteConfig.getSetting(name_of_parameter)'''

    def unload(self):
        '''Setting should be removed here, so they do not appear anymore
        when the plugin is unloaded'''
        AlgorithmProvider.unload(self)
        SextanteConfig.removeSetting( ExampleAlgorithmProvider.MY_DUMMY_SETTING)


    def getName(self):
        '''This name is  used to create the command line name of all the algorithms
        from this provider'''
        return "exampleprovider"

    def getDescription(self):
        '''This is the name that will appear on the toolbox group.'''
        return "Example algorithms"

    def getIcon(self):
        '''We return the default icon'''
        return AlgorithmProvider.getIcon(self)


    def _loadAlgorithms(self):
        '''Here we fill the list of algorithms in self.algs.
        This method is called whenever the list of algorithms should be updated.
        If the list of algorithms can change while executing SEXTANTE for QGIS
        (for instance, if it contains algorithms from user-defined scripts and
        a new script might have been added), you should create the list again
        here.
        In this case, since the list is always the same, we assign from the pre-made list.
        This assignment has to be done in this method even if the list does not change,
        since the self.algs list is cleared before calling this method'''
        self.algs = self.alglist
