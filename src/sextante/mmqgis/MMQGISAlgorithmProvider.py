import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.script.ScriptAlgorithmProvider import ScriptAlgorithmProvider
class MMQGISAlgorithmProvider(ScriptAlgorithmProvider):

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/script.png")

    def scriptsFolder(self):
        return os.path.dirname(__file__) + "/scripts"

    def getName(self):
        return "mmqgis (vector analysis)"
