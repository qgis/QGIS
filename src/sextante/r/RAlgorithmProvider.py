from PyQt4.QtCore import *
from PyQt4.QtGui import *
import os.path
from sextante.script.WrongScriptException import WrongScriptException
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.SextanteLog import SextanteLog
from sextante.core.AlgorithmProvider import AlgorithmProvider
from PyQt4 import QtGui
from sextante.r.RUtils import RUtils
from sextante.r.RAlgorithm import RAlgorithm
from sextante.r.CreateNewRScriptAction import CreateNewRScriptAction
from sextante.r.EditRScriptAction import EditRScriptAction
from sextante.core.SextanteUtils import SextanteUtils

class RAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.actions.append(CreateNewRScriptAction())
        self.contextMenuActions = [EditRScriptAction()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), RUtils.RSCRIPTS_FOLDER, "R Scripts folder", RUtils.RScriptsFolder()))
        if SextanteUtils.isWindows():
            SextanteConfig.addSetting(Setting(self.getDescription(), RUtils.R_FOLDER, "R folder", RUtils.RFolder()))

    def unload(self):
        AlgorithmProvider.unload(self)
        SextanteConfig.removeSetting(RUtils.RSCRIPTS_FOLDER)
        if SextanteUtils.isWindows():
            SextanteConfig.removeSetting(RUtils.R_FOLDER)

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/r.png")


    def getDescription(self):
        return "R scripts"

    def getName(self):
        return "r"

    def _loadAlgorithms(self):
        folder = RUtils.RScriptsFolder()
        for descriptionFile in os.listdir(folder):
            if descriptionFile.endswith("rsx"):
                try:
                    fullpath = os.path.join(RUtils.RScriptsFolder(), descriptionFile)
                    alg = RAlgorithm(fullpath)
                    if alg.name.strip() != "":
                        self.algs.append(alg)
                except WrongScriptException,e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,e.msg)
                except Exception, e:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR,"Could not load R script:" + descriptionFile)



