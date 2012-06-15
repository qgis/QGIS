import os
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.otb.OTBUtils import OTBUtils
from sextante.otb.OTBAlgorithm import OTBAlgorithm
from sextante.core.SextanteLog import SextanteLog

class OTBAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.createAlgsList()


    def getDescription(self):
        return "Orfeo Toolbox (Image analysis)"

    def getName(self):
        return "otb"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/otb.png")

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = OTBUtils.otbDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("txt"):
                try:
                    alg = OTBAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                    else:
                        SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open OTB algorithm: " + descriptionFile)
                except Exception,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open OTB algorithm: " + descriptionFile)


    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_FOLDER, "OTB command line tools folder", OTBUtils.otbPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_LIB_FOLDER, "OTB applications folder", OTBUtils.otbLibPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_SRTM_FOLDER, "SRTM tiles folder", OTBUtils.otbSRTMPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), OTBUtils.OTB_GEOID_FILE, "Geoid file", OTBUtils.otbGeoidPath()))

    def unload(self):
        AlgorithmProvider.unload(self)
        SextanteConfig.removeSetting(OTBUtils.OTB_FOLDER)
        SextanteConfig.removeSetting(OTBUtils.OTB_LIB_FOLDER)
