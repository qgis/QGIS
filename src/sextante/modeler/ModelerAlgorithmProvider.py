from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.SextanteLog import SextanteLog
from sextante.modeler.ModelerUtils import ModelerUtils
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm
from sextante.modeler.WrongModelException import WrongModelException
from sextante.modeler.EditModelAction import EditModelAction
from sextante.modeler.CreateNewModelAction import CreateNewModelAction
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui
from sextante.modeler.DeleteModelAction import DeleteModelAction

class ModelerAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.actions = [CreateNewModelAction()]
        self.contextMenuActions = [EditModelAction(), DeleteModelAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), ModelerUtils.MODELS_FOLDER, "Models folder", ModelerUtils.modelsFolder()))

    def setAlgsList(self, algs):
        ModelerUtils.allAlgs = algs

    def modelsFolder(self):
        return ModelerUtils.modelsFolder()

    def getDescription(self):
        return "Modeler"

    def getName(self):
        return "model"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")

    def _loadAlgorithms(self):
        folder = ModelerUtils.modelsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("model"):
                try:
                    alg = ModelerAlgorithm()
                    fullpath = os.path.join(ModelerUtils.modelsFolder(),descriptionFile)
                    alg.openModel(fullpath)
                    if alg.name.strip() != "":
                        alg.provider = self
                        self.algs.append(alg)
                except WrongModelException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,"Could not load model " + descriptionFile + "\n" + e.msg)