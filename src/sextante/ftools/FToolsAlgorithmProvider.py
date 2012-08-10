import os

from PyQt4 import QtGui

from sextante.core.AlgorithmProvider import AlgorithmProvider

# analysis tools
from sextante.ftools.SumLines import SumLines
from sextante.ftools.MeanCoords import MeanCoords
from sextante.ftools.UniqueValues import UniqueValues
from sextante.ftools.PointDistance import PointDistance
from sextante.ftools.BasicStatistics import BasicStatistics
from sextante.ftools.PointsInPolygon import PointsInPolygon
from sextante.ftools.LinesIntersection import LinesIntersection
from sextante.ftools.NearestNeighbourAnalysis import NearestNeighbourAnalysis

# data management tools

# geometry tools
from sextante.ftools.Delaunay import Delaunay
from sextante.ftools.Centroids import Centroids
from sextante.ftools.ExtractNodes import ExtractNodes
from sextante.ftools.VoronoiPolygons import VoronoiPolygons
from sextante.ftools.LinesToPolygons import LinesToPolygons
from sextante.ftools.PolygonsToLines import PolygonsToLines
from sextante.ftools.SimplifyGeometries import SimplifyGeometries
from sextante.ftools.ExportGeometryInfo import ExportGeometryInfo
from sextante.ftools.MultipartToSingleparts import MultipartToSingleparts
from sextante.ftools.SinglePartsToMultiparts import SinglePartsToMultiparts

# geoprocessing tools
from sextante.ftools.Clip import Clip
from sextante.ftools.Union import Union
from sextante.ftools.Dissolve import Dissolve
from sextante.ftools.ConvexHull import ConvexHull
from sextante.ftools.Difference import Difference
from sextante.ftools.Intersection import Intersection
from sextante.ftools.FixedDistanceBuffer import FixedDistanceBuffer
from sextante.ftools.VariableDistanceBuffer import VariableDistanceBuffer

# research tools
from sextante.ftools.ExtentFromLayer import ExtentFromLayer
from sextante.ftools.RandomSelection import RandomSelection
from sextante.ftools.SelectByLocation import SelectByLocation
from sextante.ftools.RandomSelectionWithinSubsets import RandomSelectionWithinSubsets

class FToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [SumLines(), PointsInPolygon(), BasicStatistics(),
                        NearestNeighbourAnalysis(), MeanCoords(), LinesIntersection(),
                        UniqueValues(), PointDistance(),
                        # data management
                        # geometry
                        ExportGeometryInfo(), Centroids(), Delaunay(), VoronoiPolygons(),
                        SimplifyGeometries(), MultipartToSingleparts(), SinglePartsToMultiparts(),
                        PolygonsToLines(), LinesToPolygons(), ExtractNodes(),
                        # geoprocessing
                        ConvexHull(), FixedDistanceBuffer(), VariableDistanceBuffer(),
                        Dissolve(), Difference(), Intersection(), Union(), Clip(),
                        # research
                        ExtentFromLayer(), RandomSelection(), RandomSelectionWithinSubsets(),
                        SelectByLocation()
                       ]

    def getDescription(self):
        return "fTools (Vector analysis)"

    def getName(self):
        return "ftools"

    def getIcon(self):
        return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/ftools_logo.png")

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def getSupportedOutputTableExtensions(self):
        return ["csv"]

    def supportsNonFileBasedOutput(self):
        return True