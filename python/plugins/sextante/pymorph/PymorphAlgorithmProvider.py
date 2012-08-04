import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.script.WrongScriptException import WrongScriptException
from sextante.core.SextanteLog import SextanteLog
from sextante.gdal.GdalUtils import GdalUtils
from sextante.pymorph.PymorphAlgorithm import PymorphAlgorithm

class PymorphAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        #self.readAlgNames()
        self.createAlgsList()

    def scriptsFolder(self):
        return os.path.dirname(__file__) + "/scripts"

    def getDescription(self):
        return "Pymorph (Morphological image processing tools)"

    def getName(self):
        return "pymorph"

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = self.scriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("py"):
                try:
                    fullpath = os.path.join(self.scriptsFolder(), descriptionFile)
                    alg = PymorphAlgorithm(fullpath)
                    alg.group = "Algorithms"
                    self.preloadedAlgs.append(alg)
                except WrongScriptException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,e.msg)

    def getSupportedOutputRasterLayerExtensions(self):
        return GdalUtils.getSupportedRasterExtensions()