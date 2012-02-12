import os
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.SagaUtils import SagaUtils
from sextante.core.SextanteUtils import SextanteUtils
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig, Setting

class SagaAlgorithmProvider():


    def __init__(self):
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_FOLDER, "SAGA folder", SagaUtils.sagaPath()))
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.ACTIVATE_SAGA, "Activate SAGA algorithms", True))
        #self.loadAlgorithms()
        self.actions = []
        self.contextMenuActions = []
        self.icon = QIcon(os.path.dirname(__file__) + "/saga.png")


    def loadAlgorithms(self):
        self.algs = []
        if not SextanteConfig.getSetting(SagaUtils.ACTIVATE_SAGA):
           return
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.startswith("alg"):
                try:
                    alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except Exception,e:
                    pass
                    #SextanteUtils.addToLog(SextanteUtils.LOG_ERROR,descriptionFile)
                    #SextanteUtils.addToLog(SextanteUtils.LOG_ERROR,str(e))

    def getName(self):
        return "SAGA"



