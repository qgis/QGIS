import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.script.WrongScriptException import WrongScriptException
from sextante.core.SextanteLog import SextanteLog
from sextante.mmqgis.MMQGISAlgorithm import MMQGISAlgorithm
class MMQGISAlgorithmProvider(AlgorithmProvider):

    def scriptsFolder(self):
        return os.path.dirname(__file__) + "/scripts"

    def getName(self):
        return "mmqgis"

    def _loadAlgorithms(self):
        folder = self.scriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    fullpath = os.path.join(self.scriptsFolder(), descriptionFile)
                    alg = MMQGISAlgorithm(fullpath)
                    alg.group = "mmqgis"
                    alg.provider = self
                    self.algs.append(alg)
                except WrongScriptException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,e.msg)
