import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteUtils import SextanteUtils
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.core.SextanteConfig import Setting, SextanteConfig
from sextante.lidar.lastools.las2dem import las2dem
from sextante.lidar.lastools.las2iso import las2iso
from sextante.lidar.lastools.las2shp import las2shp
from sextante.lidar.lastools.lasboundary import lasboundary
from sextante.lidar.lastools.lasgrid import lasgrid
from sextante.lidar.lastools.lasground import lasground
from sextante.lidar.lastools.lasclassify import lasclassify
from sextante.lidar.lastools.lasclip import lasclip
from sextante.lidar.lastools.lasheight import lasheight
from sextante.lidar.lastools.lasinfo import lasinfo
from sextante.lidar.lastools.lasprecision import lasprecision
from sextante.lidar.lastools.lassplit import lassplit
from sextante.lidar.fusion.OpenViewerAction import OpenViewerAction
from sextante.lidar.fusion.CanopyMaxima import CanopyMaxima
from sextante.lidar.fusion.CanopyModel import CanopyModel
from sextante.lidar.fusion.ClipData import ClipData
from sextante.lidar.fusion.CloudMetrics import CloudMetrics
from sextante.lidar.fusion.Cover import Cover
from sextante.lidar.fusion.GridMetrics import GridMetrics
from sextante.lidar.fusion.GridSurfaceCreate import GridSurfaceCreate
from sextante.lidar.fusion.GroundFilter import GroundFilter
from sextante.lidar.fusion.MergeData import MergeData
from sextante.lidar.fusion.FilterData import FilterData
from sextante.lidar.fusion.FusionUtils import FusionUtils


class LidarToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.algsList = []
        if SextanteUtils.isWindows():
            lastools = [las2shp(), lasboundary(), las2dem(), las2iso(), lasgrid(), lasground(),
                         lasinfo(), lasheight(), lasprecision(), lassplit(), lasclassify(), lasclip()]
        else:
            lastools = [lasinfo(), lasprecision()]
        for alg in lastools:
            alg.group = "LASTools"
        self.algsList.extend(lastools)

        if SextanteUtils.isWindows():
            self.actions.append(OpenViewerAction())
            fusiontools = [CloudMetrics(), CanopyMaxima(), CanopyModel(), ClipData(), Cover(), FilterData(),
                         GridMetrics(), GroundFilter(), GridSurfaceCreate(), MergeData()]
            for alg in fusiontools:
                alg.group = "Fusion"
            self.algsList.extend(fusiontools)

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), LasToolsUtils.LASTOOLS_FOLDER, "LASTools folder", LasToolsUtils.LasToolsPath()))
        SextanteConfig.addSetting(Setting(self.getDescription(), FusionUtils.FUSION_FOLDER, "Fusion folder",
                                          FusionUtils.FusionPath()))

    def getName(self):
        return "lidartools"

    def getDescription(self):
        return "Tools for LiDAR data"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/tool.png")

    def _loadAlgorithms(self):
        self.algs = self.algsList

    def getSupportedOutputTableExtensions(self):
        return ["csv"]