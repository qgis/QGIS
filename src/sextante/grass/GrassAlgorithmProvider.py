import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteLog import SextanteLog
from sextante.grass.GrassUtils import GrassUtils
from sextante.grass.GrassAlgorithm import GrassAlgorithm
from sextante.core.SextanteUtils import SextanteUtils

class GrassAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.createAlgsList() #preloading algorithms to speed up

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        if SextanteUtils.isWindows():
            SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_FOLDER, "GRASS folder", GrassUtils.grassPath()))
            SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_WIN_SHELL, "Msys folder", GrassUtils.grassWinShell()))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_AUTO_REGION, "Use min covering region", True))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_LATLON, "Coordinates are lat/lon", False))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_XMIN, "GRASS Region min x", 0))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_YMIN, "GRASS Region min y", 0))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_XMAX, "GRASS Region max x", 1000))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_YMAX, "GRASS Region max y", 1000))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_CELLSIZE, "GRASS Region cellsize", 1))

        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_HELP_FOLDER, "GRASS help folder", GrassUtils.grassHelpPath()))

    def unload(self):
        AlgorithmProvider.unload(self)
        if SextanteUtils.isWindows():
            SextanteConfig.removeSetting(GrassUtils.GRASS_FOLDER)
            SextanteConfig.removeSetting(GrassUtils.GRASS_WIN_SHELL)
        SextanteConfig.removeSetting(GrassUtils.GRASS_AUTO_REGION)
        SextanteConfig.removeSetting(GrassUtils.GRASS_LATLON)
        SextanteConfig.removeSetting(GrassUtils.GRASS_REGION_XMIN)
        SextanteConfig.removeSetting(GrassUtils.GRASS_REGION_YMIN)
        SextanteConfig.removeSetting(GrassUtils.GRASS_REGION_XMAX)
        SextanteConfig.removeSetting(GrassUtils.GRASS_REGION_YMAX)
        SextanteConfig.removeSetting(GrassUtils.GRASS_REGION_CELLSIZE)
        SextanteConfig.removeSetting(GrassUtils.GRASS_HELP_FOLDER)

    def createAlgsList(self):
        self.preloadedAlgs = []
        folder = GrassUtils.grassDescriptionPath()
        for descriptionFile in os.listdir(folder):
            try:
                alg = GrassAlgorithm(os.path.join(folder, descriptionFile))
                if alg.name.strip() != "":
                    self.preloadedAlgs.append(alg)
                else:
                    SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open GRASS algorithm: " + descriptionFile)
            except Exception,e:
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open GRASS algorithm: " + descriptionFile)
        self.createDescriptionFiles()

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def getName(self):
        return "GRASS"

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def createDescriptionFiles(self):
        folder = "C:\\descs\\grass"
        i = 0
        for alg in self.preloadedAlgs:
            f = open (os.path.join(folder, alg.name +".txt"), "w")
            f.write(alg.name + "\n")
            f.write(alg.name + "\n")
            f.write(alg.group + "\n")
            for param in alg.parameters:
                f.write(param.serialize() + "\n")
            for out in alg.outputs:
                f.write(out.serialize() + "\n")
            f.close()
            i+=1


