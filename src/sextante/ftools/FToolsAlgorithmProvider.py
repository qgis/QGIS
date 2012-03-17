from sextante.core.AlgorithmProvider import AlgorithmProvider

import os
from PyQt4 import QtGui
from sextante.ftools.Centroids import Centroids
from sextante.ftools.Delaunay import Delaunay
from sextante.ftools.SimplifyGeometries import SimplifyGeometries
from sextante.ftools.VoronoiPolygons import VoronoiPolygons
from sextante.ftools.ExportGeometryInfo import ExportGeometryInfo
from sextante.ftools.ExtractNodes import ExtractNodes
from sextante.ftools.LinesToPolygons import LinesToPolygons
from sextante.ftools.PolygonsToLines import PolygonsToLines
from sextante.ftools.SinglePartsToMultiparts import SinglePartsToMultiparts
from sextante.ftools.BasicStatistics import BasicStatistics
from sextante.ftools.PointsInPolygon import PointsInPolygon
from sextante.ftools.SumLines import SumLines
from sextante.ftools.MeanCoords import MeanCoords
from sextante.ftools.NearestNeighbourAnalysis import NearestNeighbourAnalysis
from sextante.ftools.LinesIntersection import LinesIntersection
from sextante.ftools.ConvexHull import ConvexHull
from sextante.ftools.FixedDistanceBuffer import FixedDistanceBuffer
from sextante.ftools.VariableDistanceBuffer import VariableDistanceBuffer
from sextante.ftools.Dissolve import Dissolve
from sextante.ftools.Difference import Difference
from sextante.ftools.Intersection import Intersection
from sextante.ftools.Union import Union
from sextante.ftools.Clip import Clip
from sextante.ftools.ExtentFromLayer import ExtentFromLayer
from sextante.ftools.RandomSelection import RandomSelection
from sextante.ftools.SelectByLocation import SelectByLocation
from sextante.ftools.RandomSelectionWithinSubsets import RandomSelectionWithinSubsets

class FToolsAlgorithmProvider(AlgorithmProvider):

    def getName(self):
        return "ftools"

    def getIcon(self):
        return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/ftools_logo.png")

    def _loadAlgorithms(self):
        self.algs = [SinglePartsToMultiparts(), PolygonsToLines(),
                     LinesToPolygons(), ExportGeometryInfo(), ExtractNodes(),
                     Centroids(), SimplifyGeometries(), Delaunay(), VoronoiPolygons(),
                     SumLines(), BasicStatistics(), PointsInPolygon(),
                     NearestNeighbourAnalysis(), MeanCoords(), LinesIntersection(),
                     ConvexHull(), FixedDistanceBuffer(), VariableDistanceBuffer(),
                     Dissolve(), Difference(), Intersection(), Union(), Clip(), ExtentFromLayer(),
                     RandomSelection(), RandomSelectionWithinSubsets(), SelectByLocation()]
        for alg in self.algs:
            alg.provider = self

    def getSupportedOutputTableExtensions(self):
        return ["csv"]
