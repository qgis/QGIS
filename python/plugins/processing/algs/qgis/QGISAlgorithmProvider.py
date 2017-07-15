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
from .ConcaveHull import ConcaveHull
from .CreateAttributeIndex import CreateAttributeIndex
from .Delaunay import Delaunay
from .DeleteColumn import DeleteColumn
from .DeleteHoles import DeleteHoles
from .DensifyGeometries import DensifyGeometries
from .DensifyGeometriesInterval import DensifyGeometriesInterval
from .Difference import Difference
from .DropGeometry import DropGeometry
from .ExtentFromLayer import ExtentFromLayer
from .FixGeometry import FixGeometry
from .GridPolygon import GridPolygon
from .Heatmap import Heatmap
from .Hillshade import Hillshade
from .ImportIntoPostGIS import ImportIntoPostGIS
from .ImportIntoSpatialite import ImportIntoSpatialite
from .Intersection import Intersection
from .LinesToPolygons import LinesToPolygons
from .Merge import Merge
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .PointsInPolygon import PointsInPolygon
from .PointsLayerFromTable import PointsLayerFromTable
from .PolygonsToLines import PolygonsToLines
from .PostGISExecuteSQL import PostGISExecuteSQL
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RegularPoints import RegularPoints
from .Ruggedness import Ruggedness
from .SaveSelectedFeatures import SaveSelectedFeatures
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .SimplifyGeometries import SimplifyGeometries
from .Slope import Slope
from .Smooth import Smooth
from .SnapGeometries import SnapGeometriesToLayer
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .SumLines import SumLines
from .SymmetricalDifference import SymmetricalDifference
from .Union import Union
from .VectorSplit import VectorSplit
from .VoronoiPolygons import VoronoiPolygons
from .ZonalStatistics import ZonalStatistics

# from .ExtractByLocation import ExtractByLocation
# from .LinesIntersection import LinesIntersection
# from .MeanCoords import MeanCoords
# from .PointDistance import PointDistance
# from .UniqueValues import UniqueValues
# from .ExportGeometryInfo import ExportGeometryInfo
# from .SinglePartsToMultiparts import SinglePartsToMultiparts
# from .ExtractNodes import ExtractNodes
# from .ConvexHull import ConvexHull
# from .FixedDistanceBuffer import FixedDistanceBuffer
# from .VariableDistanceBuffer import VariableDistanceBuffer
# from .RandomSelection import RandomSelection
# from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
# from .SelectByLocation import SelectByLocation
# from .SpatialJoin import SpatialJoin
# from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
# from .TextToFloat import TextToFloat
# from .GridLine import GridLine
# from .Gridify import Gridify
# from .HubDistancePoints import HubDistancePoints
# from .HubDistanceLines import HubDistanceLines
# from .HubLines import HubLines
# from .GeometryConvert import GeometryConvert
# from .RasterLayerStatistics import RasterLayerStatistics
# from .StatisticsByCategories import StatisticsByCategories
# from .EquivalentNumField import EquivalentNumField
# from .FieldsCalculator import FieldsCalculator
# from .Explode import Explode
# from .FieldPyculator import FieldsPyculator
# from .JoinAttributes import JoinAttributes
# from .CreateConstantRaster import CreateConstantRaster
# from .PointsDisplacement import PointsDisplacement
# from .PointsFromPolygons import PointsFromPolygons
# from .PointsFromLines import PointsFromLines
# from .RandomPointsExtent import RandomPointsExtent
# from .RandomPointsLayer import RandomPointsLayer
# from .RandomPointsPolygonsFixed import RandomPointsPolygonsFixed
# from .RandomPointsPolygonsVariable import RandomPointsPolygonsVariable
# from .RandomPointsAlongLines import RandomPointsAlongLines
# from .PointsToPaths import PointsToPaths
# from .SetVectorStyle import SetVectorStyle
# from .SetRasterStyle import SetRasterStyle
# from .SelectByAttributeSum import SelectByAttributeSum
# from .HypsometricCurves import HypsometricCurves
# from .SplitWithLines import SplitWithLines
# from .FieldsMapper import FieldsMapper
# from .Datasources2Vrt import Datasources2Vrt
# from .OrientedMinimumBoundingBox import OrientedMinimumBoundingBox
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
# from .Relief import Relief
# from .IdwInterpolation import IdwInterpolation
# from .TinInterpolation import TinInterpolation
# from .RemoveNullGeometry import RemoveNullGeometry
# from .ExtendLines import ExtendLines
# from .ExtractSpecificNodes import ExtractSpecificNodes
# from .GeometryByExpression import GeometryByExpression
# from .PoleOfInaccessibility import PoleOfInaccessibility
# from .RasterCalculator import RasterCalculator
# from .Orthogonalize import Orthogonalize
# from .ShortestPathPointToPoint import ShortestPathPointToPoint
# from .ShortestPathPointToLayer import ShortestPathPointToLayer
# from .ShortestPathLayerToPoint import ShortestPathLayerToPoint
# from .ServiceAreaFromPoint import ServiceAreaFromPoint
# from .ServiceAreaFromLayer import ServiceAreaFromLayer
# from .TruncateTable import TruncateTable
# from .Polygonize import Polygonize
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
        # algs = [MeanCoords(),
        #         LinesIntersection(), UniqueValues(), PointDistance(),
        #         ExportGeometryInfo(),
        #         SinglePartsToMultiparts(),
        #         ExtractNodes(),
        #         ConvexHull(), FixedDistanceBuffer(),
        #         VariableDistanceBuffer(),
        #         RandomSelection(), RandomSelectionWithinSubsets(),
        #         SelectByLocation(),
        #         ExtractByLocation(),
        #         SpatialJoin(),
        #         DeleteDuplicateGeometries(), TextToFloat(),
        #         GridLine(), Gridify(), HubDistancePoints(),
        #         HubDistanceLines(), HubLines(),
        #         GeometryConvert(), FieldsCalculator(),
        #          JoinAttributes(),
        #         Explode(), FieldsPyculator(),
        #         EquivalentNumField(),
        #         StatisticsByCategories(),
        #         RasterLayerStatistics(), PointsDisplacement(),
        #         PointsFromPolygons(),
        #         PointsFromLines(), RandomPointsExtent(),
        #         RandomPointsLayer(), RandomPointsPolygonsFixed(),
        #         RandomPointsPolygonsVariable(),
        #         RandomPointsAlongLines(), PointsToPaths(),
        #         SetVectorStyle(), SetRasterStyle(),
        #          HypsometricCurves(),
        #         SplitWithLines(), CreateConstantRaster(),
        #         FieldsMapper(), SelectByAttributeSum(), Datasources2Vrt(),
        #         OrientedMinimumBoundingBox(),
        #         ReverseLineDirection(), SpatialIndex(), DefineProjection(),
        #         RectanglesOvalsDiamondsVariable(),
        #         RectanglesOvalsDiamondsFixed(), MergeLines(),
        #         PointOnSurface(),
        #         OffsetLine(), Translate(),
        #         SingleSidedBuffer(), PointsAlongGeometry(),
        #          Slope(), Ruggedness(), Hillshade(),
        #         Relief(),
        #         IdwInterpolation(), TinInterpolation(),
        #         RemoveNullGeometry(),
        #         ExtendLines(), ExtractSpecificNodes(),
        #         GeometryByExpression(),
        #         PoleOfInaccessibility(),
        #
        #         RasterCalculator(), Heatmap(), Orthogonalize(),
        #         ShortestPathPointToPoint(), ShortestPathPointToLayer(),
        #         ShortestPathLayerToPoint(), ServiceAreaFromPoint(),
        #         ServiceAreaFromLayer(), TruncateTable(), Polygonize(),
        #          ExecuteSQL(), FindProjection(),
        #         TopoColor(), EliminateSelection()
        #         ]
        algs = [AddTableField(),
                Aspect(),
                AutoincrementalField(),
                BasicStatisticsForField(),
                Boundary(),
                BoundingBox(),
                CheckValidity(),
                ConcaveHull(),
                CreateAttributeIndex(),
                Delaunay(),
                DeleteColumn(),
                DeleteHoles(),
                DensifyGeometries(),
                DensifyGeometriesInterval(),
                Difference(),
                DropGeometry(),
                ExtentFromLayer(),
                FixGeometry(),
                GridPolygon(),
                Heatmap(),
                Hillshade(),
                ImportIntoPostGIS(),
                ImportIntoSpatialite(),
                Intersection(),
                LinesToPolygons(),
                Merge(),
                NearestNeighbourAnalysis(),
                PointsInPolygon(),
                PointsLayerFromTable(),
                PolygonsToLines(),
                PostGISExecuteSQL(),
                RandomExtract(),
                RandomExtractWithinSubsets(),
                RegularPoints(),
                Ruggedness(),
                SaveSelectedFeatures(),
                SelectByAttribute(),
                SelectByExpression(),
                SimplifyGeometries(),
                Slope(),
                Smooth(),
                SnapGeometriesToLayer(),
                SpatialiteExecuteSQL(),
                SumLines(),
                SymmetricalDifference(),
                Union(),
                VectorSplit(),
                VoronoiPolygons(),
                ZonalStatistics()
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
        algs.extend(scripts)

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
