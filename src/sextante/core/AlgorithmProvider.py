from sextante.core.SextanteConfig import Setting, SextanteConfig
import os
from PyQt4 import QtGui
class AlgorithmProvider():
    def __init__(self):
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        SextanteConfig.addSetting(Setting(self.getName(), name, "Activate", True))
        self.actions = []
        self.contextMenuActions = []

    def getName(self):
        return "Generic algorithm provider"

    def loadAlgorithms(self):
        self.algs = []
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        if not SextanteConfig.getSetting(name):
            return
        else:
            self._loadAlgorithms()

    #methods to be overridden.
    #==============================

    #Algorithm loading should take place here, filling sefl.algs
    #Since algorithms should have a reference to the provider they come
    #from, this is also the place to set the 'provider' variable of each algorithm
    def _loadAlgorithms(self):
        pass

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]

    def getSupportedOutputVectorLayerExtensions(self):
        return ["shp"]

    def getSupportedOutputTableExtensions(self):
        return ["dbf"]