# -*- coding: utf-8 -*-

"""
***************************************************************************
    QGISAlgorithmProvider.py
    ---------------------
    Date                 : December 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

try:
    import plotly  # NOQA
    hasPlotly = True
except:
    hasPlotly = False

from qgis.core import (QgsApplication,
                       QgsProcessingProvider)

from processing.script.ScriptUtils import ScriptUtils

from .QgisAlgorithm import QgisAlgorithm

from .AddTableField import AddTableField
from .Aspect import Aspect
from .AutoincrementalField import AutoincrementalField
from .BasicStatistics import BasicStatisticsForField
from .Boundary import Boundary
from .BoundingBox import BoundingBox
from .CheckValidity import CheckValidity
from .CreateAttributeIndex import CreateAttributeIndex
from .DeleteColumn import DeleteColumn
from .DeleteHoles import DeleteHoles
from .DensifyGeometries import DensifyGeometries
from .DensifyGeometriesInterval import DensifyGeometriesInterval
from .DropGeometry import DropGeometry
from .ExtentFromLayer import ExtentFromLayer
from .GridPolygon import GridPolygon
from .ImportIntoPostGIS import ImportIntoPostGIS
from .Merge import Merge
from .PostGISExecuteSQL import PostGISExecuteSQL

# from .RegularPoints import RegularPoints
# from .SymmetricalDifference import SymmetricalDifference
# from .VectorSplit import VectorSplit
# from .RandomExtract import RandomExtract
# from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
# from .ExtractByLocation import ExtractByLocation
# from .PointsInPolygon import PointsInPolygon
# from .PointsInPolygonUnique import PointsInPolygonUnique
# from .PointsInPolygonWeighted import PointsInPolygonWeighted
# from .SumLines import SumLines
# from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
# from .LinesIntersection import LinesIntersection
# from .MeanCoords import MeanCoords
# from .PointDistance import PointDistance
# from .UniqueValues import UniqueValues
# from .ExportGeometryInfo import ExportGeometryInfo
# from .Delaunay import Delaunay
# from .VoronoiPolygons import VoronoiPolygons
# from .SimplifyGeometries import SimplifyGeometries
# from .LinesToPolygons import LinesToPolygons
# from .PolygonsToLines import PolygonsToLines
# from .SinglePartsToMultiparts import SinglePartsToMultiparts
# from .ExtractNodes import ExtractNodes
# from .ConvexHull import ConvexHull
# from .FixedDistanceBuffer import FixedDistanceBuffer
# from .VariableDistanceBuffer import VariableDistanceBuffer
# from .Difference import Difference
# from .Intersection import Intersection
# from .RandomSelection import RandomSelection
# from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
# from .SelectByLocation import SelectByLocation
# from .Union import Union
# from .SpatialJoin import SpatialJoin
# from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
# from .TextToFloat import TextToFloat
# from .SelectByAttribute import SelectByAttribute
# from .GridLine import GridLine
# from .Gridify import Gridify
# from .HubDistancePoints import HubDistancePoints
# from .HubDistanceLines import HubDistanceLines
# from .HubLines import HubLines
# from .GeometryConvert import GeometryConvert
# from .ConcaveHull import ConcaveHull
# from .RasterLayerStatistics import RasterLayerStatistics
# from .StatisticsByCategories import StatisticsByCategories
# from .EquivalentNumField import EquivalentNumField
# from .FieldsCalculator import FieldsCalculator
# from .SaveSelectedFeatures import SaveSelectedFeatures
# from .Explode import Explode
# from .FieldPyculator import FieldsPyculator
# from .JoinAttributes import JoinAttributes
# from .CreateConstantRaster import CreateConstantRaster
# from .PointsLayerFromTable import PointsLayerFromTable
# from .PointsDisplacement import PointsDisplacement
# from .ZonalStatistics import ZonalStatistics
# from .PointsFromPolygons import PointsFromPolygons
# from .PointsFromLines import PointsFromLines
# from .RandomPointsExtent import RandomPointsExtent
# from .RandomPointsLayer import RandomPointsLayer
# from .RandomPointsPolygonsFixed import RandomPointsPolygonsFixed
# from .RandomPointsPolygonsVariable import RandomPointsPolygonsVariable
# from .RandomPointsAlongLines import RandomPointsAlongLines
# from .PointsToPaths import PointsToPaths
# from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
# from .ImportIntoSpatialite import ImportIntoSpatialite
# from .SetVectorStyle import SetVectorStyle
# from .SetRasterStyle import SetRasterStyle
# from .SelectByExpression import SelectByExpression
# from .SelectByAttributeSum import SelectByAttributeSum
# from .HypsometricCurves import HypsometricCurves
# from .SplitWithLines import SplitWithLines
# from .FieldsMapper import FieldsMapper
# from .Datasources2Vrt import Datasources2Vrt
# from .OrientedMinimumBoundingBox import OrientedMinimumBoundingBox
# from .Smooth import Smooth
# from .ReverseLineDirection import ReverseLineDirection
# from .SpatialIndex import SpatialIndex
# from .DefineProjection import DefineProjection
# from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
# from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
# from .MergeLines import MergeLines
# from .PointOnSurface import PointOnSurface
# from .OffsetLine import OffsetLine
# from .Translate import Translate
# from .SingleSidedBuffer import SingleSidedBuffer
# from .PointsAlongGeometry import PointsAlongGeometry
# from .Slope import Slope
# from .Ruggedness import Ruggedness
# from .Hillshade import Hillshade
# from .Relief import Relief
# from .IdwInterpolation import IdwInterpolation
# from .TinInterpolation import TinInterpolation
# from .ZonalStatisticsQgis import ZonalStatisticsQgis
# from .RemoveNullGeometry import RemoveNullGeometry
# from .ExtendLines import ExtendLines
# from .ExtractSpecificNodes import ExtractSpecificNodes
# from .GeometryByExpression import GeometryByExpression
# from .SnapGeometries import SnapGeometriesToLayer
# from .PoleOfInaccessibility import PoleOfInaccessibility
# from .RasterCalculator import RasterCalculator
# from .Heatmap import Heatmap
# from .Orthogonalize import Orthogonalize
# from .ShortestPathPointToPoint import ShortestPathPointToPoint
# from .ShortestPathPointToLayer import ShortestPathPointToLayer
# from .ShortestPathLayerToPoint import ShortestPathLayerToPoint
# from .ServiceAreaFromPoint import ServiceAreaFromPoint
# from .ServiceAreaFromLayer import ServiceAreaFromLayer
# from .TruncateTable import TruncateTable
# from .Polygonize import Polygonize
# from .FixGeometry import FixGeometry
# from .ExecuteSQL import ExecuteSQL
# from .FindProjection import FindProjection
# from .TopoColors import TopoColor
# from .EliminateSelection import EliminateSelection

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class QGISAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []
        self.externalAlgs = []

    def getAlgs(self):
        # algs = [SumLines(), PointsInPolygon(),
        #         PointsInPolygonWeighted(), PointsInPolygonUnique(),
        #         NearestNeighbourAnalysis(), MeanCoords(),
        #         LinesIntersection(), UniqueValues(), PointDistance(),
        #         ExportGeometryInfo(),
        #         Delaunay(), VoronoiPolygons(), SimplifyGeometries(),
        #         , SinglePartsToMultiparts(),
        #         PolygonsToLines(), LinesToPolygons(), ExtractNodes(),
        #         ConvexHull(), FixedDistanceBuffer(),
        #         VariableDistanceBuffer(), Difference(),
        #         Intersection(), Union(),
        #         RandomSelection(), RandomSelectionWithinSubsets(),
        #         SelectByLocation(), RandomExtract(),
        #         RandomExtractWithinSubsets(), ExtractByLocation(),
        #         SpatialJoin(), RegularPoints(), SymmetricalDifference(),
        #         VectorSplit(),
        #         DeleteDuplicateGeometries(), TextToFloat(),
        #         SelectByAttribute(),
        #         GridLine(), Gridify(), HubDistancePoints(),
        #         HubDistanceLines(), HubLines(),
        #         GeometryConvert(), FieldsCalculator(),
        #         SaveSelectedFeatures(), JoinAttributes(),
        #         Explode(), FieldsPyculator(),
        #         EquivalentNumField(), PointsLayerFromTable(),
        #         StatisticsByCategories(), ConcaveHull(),
        #         RasterLayerStatistics(), PointsDisplacement(),
        #         ZonalStatistics(), PointsFromPolygons(),
        #         PointsFromLines(), RandomPointsExtent(),
        #         RandomPointsLayer(), RandomPointsPolygonsFixed(),
        #         RandomPointsPolygonsVariable(),
        #         RandomPointsAlongLines(), PointsToPaths(),
        #         SpatialiteExecuteSQL(), ImportIntoSpatialite(),
        #         SetVectorStyle(), SetRasterStyle(),
        #         SelectByExpression(), HypsometricCurves(),
        #         SplitWithLines(), CreateConstantRaster(),
        #         FieldsMapper(), SelectByAttributeSum(), Datasources2Vrt(),
        #         OrientedMinimumBoundingBox(), Smooth(),
        #         ReverseLineDirection(), SpatialIndex(), DefineProjection(),
        #         RectanglesOvalsDiamondsVariable(),
        #         RectanglesOvalsDiamondsFixed(), MergeLines(),
        #         PointOnSurface(),
        #         OffsetLine(), Translate(),
        #         SingleSidedBuffer(), PointsAlongGeometry(),
        #          Slope(), Ruggedness(), Hillshade(),
        #         Relief(), ZonalStatisticsQgis(),
        #         IdwInterpolation(), TinInterpolation(),
        #         RemoveNullGeometry(),
        #         ExtendLines(), ExtractSpecificNodes(),
        #         GeometryByExpression(), SnapGeometriesToLayer(),
        #         PoleOfInaccessibility(),
        #
        #         RasterCalculator(), Heatmap(), Orthogonalize(),
        #         ShortestPathPointToPoint(), ShortestPathPointToLayer(),
        #         ShortestPathLayerToPoint(), ServiceAreaFromPoint(),
        #         ServiceAreaFromLayer(), TruncateTable(), Polygonize(),
        #         FixGeometry(), ExecuteSQL(), FindProjection(),
        #         TopoColor(), EliminateSelection()
        #         ]
        algs = [AddTableField(),
                Aspect(),
                AutoincrementalField(),
                BasicStatisticsForField(),
                Boundary(),
                BoundingBox(),
                CheckValidity(),
                CreateAttributeIndex(),
                DeleteColumn(),
                DeleteHoles(),
                DensifyGeometries(),
                DensifyGeometriesInterval(),
                DropGeometry(),
                ExtentFromLayer(),
                GridPolygon(),
                ImportIntoPostGIS(),
                Merge(),
                PostGISExecuteSQL()
                ]

        if hasPlotly:
            #     from .VectorLayerHistogram import VectorLayerHistogram
            #     from .RasterLayerHistogram import RasterLayerHistogram
            #     from .VectorLayerScatterplot import VectorLayerScatterplot
            #     from .MeanAndStdDevPlot import MeanAndStdDevPlot
            from .BarPlot import BarPlot
        #     from .PolarPlot import PolarPlot
        #     from .BoxPlot import BoxPlot
        #     from .VectorLayerScatterplot3D import VectorLayerScatterplot3D
        #
            algs.extend([BarPlot()])
            #[VectorLayerHistogram(), RasterLayerHistogram(),
        #                  VectorLayerScatterplot(), MeanAndStdDevPlot(),
        #                  BarPlot(), PolarPlot(), BoxPlot(),
        #                  VectorLayerScatterplot3D()])

        # to store algs added by 3rd party plugins as scripts
        folder = os.path.join(os.path.dirname(__file__), 'scripts')
        scripts = ScriptUtils.loadFromFolder(folder)
        for script in scripts:
            script.allowEdit = False
        #algs.extend(scripts)

        return algs

    def id(self):
        return 'qgis'

    def name(self):
        return 'QGIS'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def loadAlgorithms(self):
        self.algs = self.getAlgs()
        for a in self.algs:
            self.addAlgorithm(a)
        for a in self.externalAlgs:
            self.addAlgorithm(a)

    def supportsNonFileBasedOutput(self):
        return True
