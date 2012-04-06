from PyQt4.QtCore import *
import subprocess
from sextante.core.SextanteLog import SextanteLog
import os

class GdalUtils():

    @staticmethod
    def runGdal(commands, progress):
        settings = QSettings()
        path = str(settings.value( "/GdalTools/gdalPath", QVariant( "" ) ).toString())
        envval = str(os.getenv("PATH"))
        if not path.lower() in envval.lower().split(os.pathsep):
            envval += "%s%s" % (os.pathsep, str(path))
            os.putenv( "PATH", envval )
        loglines = []
        loglines.append("GDAL execution console output")
        proc = subprocess.Popen(commands, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=False).stdout
        for line in iter(proc.readline, ""):
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        GdalUtils.consoleOutput = loglines

    @staticmethod
    def getConsoleOutput():
        return GdalUtils.consoleOutput


