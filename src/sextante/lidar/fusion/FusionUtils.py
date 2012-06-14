from PyQt4.QtCore import *
from PyQt4.QtGui import *
import subprocess
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig
import os
from sextante.core.SextanteUtils import SextanteUtils

class FusionUtils():

    FUSION_FOLDER = "FUSION_FOLDER"

    @staticmethod
    def FusionPath():
        folder = SextanteConfig.getSetting(FusionUtils.FUSION_FOLDER)
        if folder == None:
            folder =""

        return folder

    @staticmethod
    def tempFileListFilepath():
        filename = "fusion_files_list.txt";
        filepath = SextanteUtils.userFolder() + os.sep + filename
        return filepath

    @staticmethod
    def createFileList(files):
        out = open(FusionUtils.tempFileListFilepath(), "w")
        for f in files:
            out.write(f + "\n")
        out.close()

    @staticmethod
    def runFusion(commands, progress):
        loglines = []
        loglines.append("Fusion execution console output")
        proc = subprocess.Popen(commands, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=False).stdout
        for line in iter(proc.readline, ""):
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)