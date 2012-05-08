import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.SextanteConfig import Setting, SextanteConfig
from sextante.fusion.FusionUtils import FusionUtils
from sextante.fusion.OpenViewerAction import OpenViewerAction
from sextante.fusion.CloudMetrics import CloudMetrics
from sextante.fusion.CanopyMaxima import CanopyMaxima
from sextante.fusion.CanopyModel import CanopyModel
from sextante.fusion.ClipData import ClipData
from sextante.fusion.Cover import Cover
from sextante.fusion.FilterData import FilterData


class FusionAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.actions.append(OpenViewerAction())
        self.algsList = [CloudMetrics(), CanopyMaxima(), CanopyModel(), ClipData(), Cover(), FilterData()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        SextanteConfig.addSetting(Setting(self.getDescription(), FusionUtils.FUSION_FOLDER, "Fusion folder", FusionUtils.FusionPath()))

    def getName(self):
        return "fusion"

    def getDescription(self):
        return "FUSION (Tools for LiDAR data)"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/tool.png")

    def _loadAlgorithms(self):
        self.algs = self.algsList

    def getSupportedOutputTableExtensions(self):
        return ["csv"]