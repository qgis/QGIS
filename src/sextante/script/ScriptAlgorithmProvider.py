from sextante.script.CreateNewScriptAction import CreateNewScriptAction
from sextante.script.EditScriptAction import EditScriptAction
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.script.DeleteScriptAction import DeleteScriptAction
from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from sextante.script.ScriptUtils import ScriptUtils
from sextante.script.WrongScriptException import WrongScriptException
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.SextanteLog import SextanteLog
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui

class ScriptAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        SextanteConfig.addSetting(Setting("Scripts", ScriptUtils.SCRIPTS_FOLDER, "Scripts folder", ScriptUtils.scriptsFolder()))
        #SextanteConfig.addSetting(Setting("Scripts", ScriptUtils.ACTIVATE_SCRIPTS, "Activate scripts", True))
        #self.actions = []
        self.actions.append(CreateNewScriptAction())
        self.contextMenuActions = [EditScriptAction(), DeleteScriptAction()]

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/script.png")

    def scriptsFolder(self):
        return ScriptUtils.scriptsFolder()

    def getName(self):
        return "Scripts"

    def _loadAlgorithms(self):
        folder = self.scriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    fullpath = os.path.join(ScriptUtils.scriptsFolder(), descriptionFile)
                    alg = ScriptAlgorithm(fullpath)
                    if alg.name.strip() != "":
                        alg.provider = self
                        self.algs.append(alg)
                except WrongScriptException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,e.msg)



