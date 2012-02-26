from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.ftools.SinglePartsToMultipartsAlgorithm import SinglePartsToMultipartsAlgorithm
from sextante.ftools.PolygonsToLinesAlgorithm import PolygonsToLinesAlgorithm
from sextante.ftools.LinesToPolygonsAlgorithm import LinesToPolygonsAlgorithm
from sextante.ftools.ExportGeometryInfoAlgorithm import ExportGeometryInfoAlgorithm
from sextante.ftools.ExtractNodesAlgorithm import ExtractNodesAlgorithm
from sextante.ftools.CentroidsAlgorithm import CentroidsAlgorithm
from sextante.ftools.SimplifyGeometriesAlgorithm import SimplifyGeometriesAlgorithm
from sextante.ftools.DelaunayAlgorithm import DelaunayAlgorithm
from sextante.ftools.SumLinesAlgorithm import SumLinesAlgorithm
from sextante.ftools.VoronoiAlgorithm import VoronoiAlgorithm

class FToolsAlgorithmProvider(AlgorithmProvider):

    def getName(self):
        return "ftools"

    def _loadAlgorithms(self):
        self.algs = [SinglePartsToMultipartsAlgorithm(), PolygonsToLinesAlgorithm(),
                     LinesToPolygonsAlgorithm(), ExportGeometryInfoAlgorithm(), ExtractNodesAlgorithm(),
                     CentroidsAlgorithm(), SimplifyGeometriesAlgorithm(), DelaunayAlgorithm(), VoronoiAlgorithm(),
                     SumLinesAlgorithm()]
        for alg in self.algs:
            alg.provider = self

    def getSupportedOutputTableExtensions(self):
        return ["csv"]
