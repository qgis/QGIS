import os
from sextante.saga.SagaAlgorithm import SagaAlgorithm
from sextante.saga.SagaUtils import SagaUtils
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui

class SagaAlgorithmProvider(AlgorithmProvider):


    def __init__(self):
        AlgorithmProvider.__init__(self)
        SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_FOLDER, "SAGA folder", SagaUtils.sagaPath()))
        #SextanteConfig.addSetting(Setting("SAGA", SagaUtils.ACTIVATE_SAGA, "Activate SAGA algorithms", True))
        #self.loadAlgorithms()
        #self.actions = []
        #self.contextMenuActions = []


    def _loadAlgorithms(self):
        folder = SagaUtils.sagaDescriptionPath()
        for descriptionFile in os.listdir(folder):
            try:
                alg = SagaAlgorithm(os.path.join(folder, descriptionFile))
                if alg.name.strip() != "":
                    alg.provider = self
                    self.algs.append(alg)
            except Exception,e:
                pass
                    #SextanteUtils.addToLog(SextanteUtils.LOG_ERROR,descriptionFile)
                    #SextanteUtils.addToLog(SextanteUtils.LOG_ERROR,str(e))
        #self.createDescriptionFiles()

    def getName(self):
        return "SAGA"

    def createDescriptionFiles(self):
        folder = SagaUtils.sagaDescriptionPath()
        i = 0
        for alg in self.algs:
            f = open (os.path.join(folder, str(i)+".txt"), "w")
            f.write(alg.name + "\n")
            f.write(alg.group + "\n")
            for param in alg.parameters:
                f.write(param.serialize() + "\n")
            for out in alg.outputs:
                f.write(out.serialize() + "\n")
            f.close()
            i+=1



