from sextante.script.CreateNewScriptAction import CreateNewScriptAction
from sextante.script.EditScriptAction import EditScriptAction
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.script.DeleteScriptAction import DeleteScriptAction
from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.core.SextanteUtils import SextanteUtils
from sextante.script.ScriptUtils import ScriptUtils

class ScriptAlgorithmProvider:

    def __init__(self):
        self.loadAlgorithms()
        self.actions = []
        self.actions.append(CreateNewScriptAction())
        self.contextMenuActions = [EditScriptAction(), DeleteScriptAction()]
        self.icon = self.getIcon()

    #This 3 methods should be subclasses to create a custom group of scripts
    #==========================================================================
    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/script.png")

    def scriptsFolder(self):
        return ScriptUtils.scriptsFolder()

    def getName(self):
        return "Scripts"
    #==========================================================================

    def loadAlgorithms(self):
        self.algs = []
        folder = self.scriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    alg = ScriptAlgorithm(descriptionFile)
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except Exception,e:
                    SextanteUtils.addToLog(SextanteUtils.LOG_ERROR,e.msg)



