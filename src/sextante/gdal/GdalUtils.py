from PyQt4.QtCore import *
from PyQt4.QtGui import *
import subprocess
from sextante.core.SextanteLog import SextanteLog
import os
import gdal

class GdalUtils():

    supportedRasters = None

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
        fused_command = ''.join(['%s ' % c for c in commands])
        proc = subprocess.Popen(fused_command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=False).stdout
        for line in iter(proc.readline, ""):
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        GdalUtils.consoleOutput = loglines

    @staticmethod
    def getConsoleOutput():
        return GdalUtils.consoleOutput

    @staticmethod
    def getSupportedRasters():
        '''this has been adapted from GdalTools plugin'''
        if GdalUtils.supportedRasters != None:
            return GdalUtils.supportedRasters

        if gdal.GetDriverCount() == 0:
            gdal.AllRegister()

        GdalUtils.supportedRasters = {}
        GdalUtils.supportedRasters["GTiff"] = ["tif"]
        for i in range(gdal.GetDriverCount()):
            driver = gdal.GetDriver(i)
            if driver == None:
                continue

            shortName = str(QString(driver.ShortName).remove( QRegExp( '\(.*$' ) ).trimmed())
            metadata = driver.GetMetadata()
            if metadata.has_key(gdal.DMD_EXTENSION):
                extensions = metadata[gdal.DMD_EXTENSION].split("/")
                if extensions:
                    GdalUtils.supportedRasters[shortName] = extensions

        return GdalUtils.supportedRasters

    @staticmethod
    def getSupportedRasterExtensions():
        allexts = ["tif"]
        for exts in GdalUtils.getSupportedRasters().values():
            for ext in exts:
                if ext not in allexts:
                    allexts.append(ext)
        return allexts

    @staticmethod
    def getFormatShortNameFromFilename(filename):
        ext = filename[filename.rfind(".")+1:]
        supported = GdalUtils.getSupportedRasters()
        for name in supported.keys():
            exts = supported[name]
            if ext in exts:
                return name
        return "GTiff"
