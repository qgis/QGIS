from sextante.script.CreateNewScriptAction import CreateNewScriptAction
from sextante.script.EditScriptAction import EditScriptAction
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path

class ScriptAlgorithmProvider:

    def __init__(self):
        self.algs = []
        self.loadAlgorithms()
        self.actions = []
        self.actions.append(CreateNewScriptAction())
        self.contextMenuActions = [EditScriptAction()]
        self.icon = QIcon(os.path.dirname(__file__) + "/script.png")

    def loadAlgorithms(self):
        pass
        #TODO!!!!!!

    def getName(self):
        return "Scripts"
