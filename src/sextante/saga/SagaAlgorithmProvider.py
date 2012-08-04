import os
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.SagaUtils import SagaUtils
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils

class SagaAlgorithmProvider(AlgorithmProvider):


    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.createAlgsList() #preloading algorithms to speed up

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if SextanteUtils.isWindows():
            SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_FOLDER, "SAGA folder", SagaUtils.sagaPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_AUTO_RESAMPLING, "Use min covering grid system for resampling", True))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_LOG_COMMANDS, "Log execution commands", False))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_LOG_CONSOLE, "Log console output", False))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_XMIN, "Resampling region min x", 0))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_YMIN, "Resampling region min y", 0))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_XMAX, "Resampling region max x", 1000))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_YMAX, "Resampling region max y", 1000))
        SextanteConfig.addSetting(Setting(self.getDescription(), SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE, "Resampling region cellsize", 1))

    def unload(self):
        AlgorithmProvider.unload(self)
        if SextanteUtils.isWindows():
            SextanteConfig.removeSetting(SagaUtils.SAGA_FOLDER)
        SextanteConfig.removeSetting(SagaUtils.SAGA_AUTO_RESAMPLING)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMIN)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMIN)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMAX)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMAX)
        SextanteConfig.removeSetting(SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE)
        SextanteConfig.removeSetting(SagaUtils.SAGA_LOG_CONSOLE)
        SextanteConfig.removeSetting(SagaUtils.SAGA_LOG_COMMANDS)

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("txt"):
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
                    else:
                        SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open SAGA algorithm: " + descriptionFile)
                except Exception,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open SAGA algorithm: " + descriptionFile)

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def getDescription(self):
        return "SAGA"

    def getName(self):
        return "saga"

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/saga.png")

    def createDescriptionFiles(self):
        folder = "C:\\descs\\saga"
        i = 0
        for alg in self.preloadedAlgs:
            f = open (os.path.join(folder, alg.name.replace(" ","").replace("/", "") + ".txt"), "w")
            f.write(alg.name + "\n")
            f.write(alg.undecoratedGroup + "\n")
            for param in alg.parameters:
                f.write(param.serialize() + "\n")
            for out in alg.outputs:
                f.write(out.serialize() + "\n")
            f.close()
            i+=1





