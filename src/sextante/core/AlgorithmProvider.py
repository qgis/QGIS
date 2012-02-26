from sextante.core.SextanteConfig import Setting, SextanteConfig
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

    #method to be overriden. Algorithm loading should take place here
    def _loadAlgorithms(self):
        pass

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]

    def getSupportedOutputVectorLayerExtensions(self):
        return ["shp"]

    def getSupportedOutputTableExtensions(self):
        return ["dbf"]