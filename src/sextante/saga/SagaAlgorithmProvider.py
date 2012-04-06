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
        self.createAlgsList() #preloading algorithms to speed up

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if SextanteUtils.isWindows():
            SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_FOLDER, "SAGA folder", SagaUtils.sagaPath()))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_AUTO_RESAMPLING, "Use min covering grid system for resampling", True))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_RESAMPLING_REGION_XMIN, "Resampling region min x", 0))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_RESAMPLING_REGION_YMIN, "Resampling region min y", 0))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_RESAMPLING_REGION_XMAX, "Resampling region max x", 1000))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_RESAMPLING_REGION_YMAX, "Resampling region max y", 1000))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE, "Resampling region cellsize", 1))

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

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            try:
                if descriptionFile.startswith("alg_"):
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.preloadedAlgs.append(alg)
            except Exception,e:
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open SAGA algorithm: " + descriptionFile)

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def getName(self):
        return "SAGA"

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/saga.png")

    def createDescriptionFiles(self):
        folder = SagaUtils.sagaDescriptionPath()
        i = 0
        for alg in self.algs:
            f = open (os.path.join(folder, "alg_" + str(i)+".txt"), "w")
            f.write(alg.name + "\n")
            f.write(alg.undecoratedGroup + "\n")
            for param in alg.parameters:
                f.write(param.serialize() + "\n")
            for out in alg.outputs:
                f.write(out.serialize() + "\n")
            f.close()
            i+=1





