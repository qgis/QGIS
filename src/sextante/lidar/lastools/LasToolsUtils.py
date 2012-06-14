from PyQt4.QtCore import *
from PyQt4.QtGui import *
import subprocess
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig

class LasToolsUtils():

    LASTOOLS_FOLDER = "LASTOOLS_FOLDER"

    @staticmethod
    def LasToolsPath():
        folder = SextanteConfig.getSetting(LasToolsUtils.LASTOOLS_FOLDER)
        if folder == None:
            folder =""

        return folder

    @staticmethod
    def runLasTools(commands, progress):
        loglines = []
        loglines.append("LasTools execution console output")
        commandline = " ".join(commands)
        proc = subprocess.Popen(commandline, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=False).stdout
        for line in iter(proc.readline, ""):
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)