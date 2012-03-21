import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig, Setting
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteLog import SextanteLog
from sextante.grass.GrassUtils import GrassUtils
from sextante.grass.GrassAlgorithm import GrassAlgorithm

class GrassAlgorithmProvider(AlgorithmProvider):


    def __init__(self):
        AlgorithmProvider.__init__(self)
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_FOLDER, "GRASS folder", GrassUtils.grassPath()))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_AUTO_REGION, "Use min covering region", True))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_LATLON, "Coordinates are lat/lon", False))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_XMIN, "GRASS Region min x", 0))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_YMIN, "GRASS Region min y", 0))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_XMAX, "GRASS Region max x", 1000))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_YMAX, "GRASS Region max y", 1000))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_REGION_CELLSIZE, "GRASS Region cellsize", 1))
        SextanteConfig.addSetting(Setting("GRASS", GrassUtils.GRASS_WIN_SHELL, "Shell executable (Windows only)", GrassUtils.grassWinShell()))
        #SextanteConfig.addSetting(Setting("SAGA", SagaUtils.SAGA_USE_SELECTED, "Use only selected features in vector layers", False))

    def _loadAlgorithms(self):
        folder = GrassUtils.grassDescriptionPath()
        for descriptionFile in os.listdir(folder):
            try:
                if descriptionFile.startswith("alg_"):
                    alg = GrassAlgorithm(os.path.join(folder, descriptionFile))
                    if alg.name.strip() != "":
                        alg.provider = self
                        self.algs.append(alg)
            except Exception,e:
                pass

    def getName(self):
        return "GRASS"


    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif", "asc"]

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/grass.png")




