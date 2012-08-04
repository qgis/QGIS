from sextante.core.SextanteConfig import Setting, SextanteConfig
import os
from PyQt4 import QtGui

class AlgorithmProvider():
    '''this is the base class for algorithms providers.
    An algorithm provider is a set of related algorithms, typically from the same
    external application or related to a common area of analysis.
    '''

    def __init__(self):
        #indicates if the provider should be active by default.
        #For provider relying on an external software, this should be
        #false, so the user should activate them manually and install
        #the required software in advance.
        self.activate = True
        self.actions = []
        self.contextMenuActions = []


    def loadAlgorithms(self):
        self.algs = []
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        if not SextanteConfig.getSetting(name):
            return
        else:
            self._loadAlgorithms()
            for alg in self.algs:
                alg.provider = self

    #methods to be overridden.
    #==============================

    def _loadAlgorithms(self):
        '''Algorithm loading should take place here, filling self.algs, which is a list of
        elements of class GeoAlgorithm. Use that class to create your own algorithms'''
        pass

    def initializeSettings(self):
        '''this is the place where you should add config parameters to SEXTANTE using the SextanteConfig class.
        this method is called when a provider is added to SEXTANTE.
        By default it just adds a setting to activate or deactivate algorithms from the provider'''
        SextanteConfig.settingIcons[self.getDescription()] = self.getIcon()
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        SextanteConfig.addSetting(Setting(self.getDescription(), name, "Activate", self.activate))

    def unload(self):
        '''Do here anything that you want to be done when the provider is removed from the list of available ones.
        This method is called when you remove the provider from Sextante.
        Removal of config setting should be done here'''
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        SextanteConfig.removeSetting(name)

    def getName(self):
        '''Returns the name to use to create the command-line name. Should be a short descriptive name of the provider'''
        return "sextante"

    def getDescription(self):
        '''Returns the full name of the provider'''
        return "Generic algorithm provider"


    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]

    def getSupportedOutputVectorLayerExtensions(self):
        return ["shp"]

    def getSupportedOutputTableExtensions(self):
        return ["dbf"]