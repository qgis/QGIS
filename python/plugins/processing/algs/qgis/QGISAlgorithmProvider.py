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
from .Aggregate import Aggregate
from .Aspect import Aspect
from .AutoincrementalField import AutoincrementalField
from .BasicStatistics import BasicStatisticsForField
from .Boundary import Boundary
from .BoundingBox import BoundingBox
from .CheckValidity import CheckValidity
from .ConcaveHull import ConcaveHull
from .ConvexHull import ConvexHull
from .CreateAttributeIndex import CreateAttributeIndex
from .CreateConstantRaster import CreateConstantRaster
from .Datasources2Vrt import Datasources2Vrt
from .DefineProjection import DefineProjection
from .Delaunay import Delaunay
from .DeleteColumn import DeleteColumn
from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
from .DeleteHoles import DeleteHoles
from .DensifyGeometries import DensifyGeometries
from .DensifyGeometriesInterval import DensifyGeometriesInterval
from .Difference import Difference
from .DropGeometry import DropGeometry
from .DropMZValues import DropMZValues
from .EliminateSelection import EliminateSelection
from .EquivalentNumField import EquivalentNumField
from .ExecuteSQL import ExecuteSQL
from .Explode import Explode
from .ExportGeometryInfo import ExportGeometryInfo
from .ExtendLines import ExtendLines
from .ExtentFromLayer import ExtentFromLayer
from .ExtractNodes import ExtractNodes
from .ExtractSpecificNodes import ExtractSpecificNodes
from .FieldPyculator import FieldsPyculator
from .FieldsCalculator import FieldsCalculator
from .FieldsMapper import FieldsMapper
from .FindProjection import FindProjection
from .FixedDistanceBuffer import FixedDistanceBuffer
from .FixGeometry import FixGeometry
from .GeometryConvert import GeometryConvert
from .GeometryByExpression import GeometryByExpression
from .Gridify import Gridify
from .GridLine import GridLine
from .GridPolygon import GridPolygon
from .Heatmap import Heatmap
from .Hillshade import Hillshade
from .HubDistanceLines import HubDistanceLines
from .HubDistancePoints import HubDistancePoints
from .HubLines import HubLines
from .HypsometricCurves import HypsometricCurves
from .IdwInterpolation import IdwInterpolation
from .ImportIntoPostGIS import ImportIntoPostGIS
from .ImportIntoSpatialite import ImportIntoSpatialite
from .Intersection import Intersection
from .JoinAttributes import JoinAttributes
from .LinesIntersection import LinesIntersection
from .LinesToPolygons import LinesToPolygons
from .MeanCoords import MeanCoords
from .Merge import Merge
from .MergeLines import MergeLines
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .OffsetLine import OffsetLine
from .OrientedMinimumBoundingBox import OrientedMinimumBoundingBox
from .Orthogonalize import Orthogonalize
from .PointDistance import PointDistance
from .PointOnSurface import PointOnSurface
from .PointsAlongGeometry import PointsAlongGeometry
from .PointsDisplacement import PointsDisplacement
from .PointsFromLines import PointsFromLines
from .PointsFromPolygons import PointsFromPolygons
from .PointsInPolygon import PointsInPolygon
from .PointsLayerFromTable import PointsLayerFromTable
from .PointsToPaths import PointsToPaths
from .PoleOfInaccessibility import PoleOfInaccessibility
from .Polygonize import Polygonize
from .PolygonsToLines import PolygonsToLines
from .PostGISExecuteSQL import PostGISExecuteSQL
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RandomPointsAlongLines import RandomPointsAlongLines
from .RandomPointsExtent import RandomPointsExtent
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygons import RandomPointsPolygons
from .RandomSelection import RandomSelection
from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from .Rasterize import RasterizeAlgorithm
from .RasterCalculator import RasterCalculator
from .RasterLayerStatistics import RasterLayerStatistics
from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
from .RegularPoints import RegularPoints
from .Relief import Relief
from .ReverseLineDirection import ReverseLineDirection
from .Ruggedness import Ruggedness
from .SaveSelectedFeatures import SaveSelectedFeatures
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .ServiceAreaFromLayer import ServiceAreaFromLayer
from .ServiceAreaFromPoint import ServiceAreaFromPoint
from .SetMValue import SetMValue
from .SetRasterStyle import SetRasterStyle
from .SetVectorStyle import SetVectorStyle
from .SetZValue import SetZValue
from .ShortestPathLayerToPoint import ShortestPathLayerToPoint
from .ShortestPathPointToLayer import ShortestPathPointToLayer
from .ShortestPathPointToPoint import ShortestPathPointToPoint
from .SimplifyGeometries import SimplifyGeometries
from .SinglePartsToMultiparts import SinglePartsToMultiparts
from .SingleSidedBuffer import SingleSidedBuffer
from .Slope import Slope
from .Smooth import Smooth
from .SnapGeometries import SnapGeometriesToLayer
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .SpatialIndex import SpatialIndex
from .SplitWithLines import SplitWithLines
from .StatisticsByCategories import StatisticsByCategories
from .SumLines import SumLines
from .SymmetricalDifference import SymmetricalDifference
from .TextToFloat import TextToFloat
from .TinInterpolation import TinInterpolation
from .TopoColors import TopoColor
from .Translate import Translate
from .TruncateTable import TruncateTable
from .Union import Union
from .UniqueValues import UniqueValues
from .VariableDistanceBuffer import VariableDistanceBuffer
from .VectorSplit import VectorSplit
from .VoronoiPolygons import VoronoiPolygons
from .ZonalStatistics import ZonalStatistics

# from .ExtractByLocation import ExtractByLocation
# from .SelectByLocation import SelectByLocation
# from .SpatialJoin import SpatialJoin
# from .SelectByAttributeSum import SelectByAttributeSum

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class QGISAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []
        self.externalAlgs = []

    def getAlgs(self):
        # algs = [
        #         SelectByLocation(),
        #         ExtractByLocation(),
        #         SpatialJoin(),
        #         SelectByAttributeSum()
        #         ]
        algs = [AddTableField(),
                Aggregate(),
                Aspect(),
                AutoincrementalField(),
                BasicStatisticsForField(),
                Boundary(),
                BoundingBox(),
                CheckValidity(),
                ConcaveHull(),
                ConvexHull(),
                CreateAttributeIndex(),
                CreateConstantRaster(),
                Datasources2Vrt(),
                DefineProjection(),
                Delaunay(),
                DeleteColumn(),
                DeleteDuplicateGeometries(),
                DeleteHoles(),
                DensifyGeometries(),
                DensifyGeometriesInterval(),
                Difference(),
                DropGeometry(),
                DropMZValues(),
                EliminateSelection(),
                EquivalentNumField(),
                ExecuteSQL(),
                Explode(),
                ExportGeometryInfo(),
                ExtendLines(),
                ExtentFromLayer(),
                ExtractNodes(),
                ExtractSpecificNodes(),
                FieldsCalculator(),
                FieldsMapper(),
                FieldsPyculator(),
                FindProjection(),
                FixedDistanceBuffer(),
                FixGeometry(),
                GeometryByExpression(),
                GeometryConvert(),
                Gridify(),
                GridLine(),
                GridPolygon(),
                Heatmap(),
                Hillshade(),
                HubDistanceLines(),
                HubDistancePoints(),
                HubLines(),
                HypsometricCurves(),
                IdwInterpolation(),
                ImportIntoPostGIS(),
                ImportIntoSpatialite(),
                Intersection(),
                JoinAttributes(),
                LinesIntersection(),
                LinesToPolygons(),
                MeanCoords(),
                Merge(),
                MergeLines(),
                NearestNeighbourAnalysis(),
                OffsetLine(),
                OrientedMinimumBoundingBox(),
                Orthogonalize(),
                PointDistance(),
                PointOnSurface(),
                PointsAlongGeometry(),
                PointsDisplacement(),
                PointsFromLines(),
                PointsFromPolygons(),
                PointsInPolygon(),
                PointsLayerFromTable(),
                PointsToPaths(),
                PoleOfInaccessibility(),
                Polygonize(),
                PolygonsToLines(),
                PostGISExecuteSQL(),
                RandomExtract(),
                RandomExtractWithinSubsets(),
                RandomPointsAlongLines(),
                RandomPointsExtent(),
                RandomPointsLayer(),
                RandomPointsPolygons(),
                RandomSelection(),
                RandomSelectionWithinSubsets(),
                RasterCalculator(),
                RasterizeAlgorithm(),
                RasterLayerStatistics(),
                RectanglesOvalsDiamondsFixed(),
                RectanglesOvalsDiamondsVariable(),
                RegularPoints(),
                Relief(),
                ReverseLineDirection(),
                Ruggedness(),
                SaveSelectedFeatures(),
                SelectByAttribute(),
                SelectByExpression(),
                ServiceAreaFromLayer(),
                ServiceAreaFromPoint(),
                SetMValue(),
                SetRasterStyle(),
                SetVectorStyle(),
                SetZValue(),
                ShortestPathLayerToPoint(),
                ShortestPathPointToLayer(),
                ShortestPathPointToPoint(),
                SimplifyGeometries(),
                SinglePartsToMultiparts(),
                SingleSidedBuffer(),
                Slope(),
                Smooth(),
                SnapGeometriesToLayer(),
                SpatialiteExecuteSQL(),
                SpatialIndex(),
                SplitWithLines(),
                StatisticsByCategories(),
                SumLines(),
                SymmetricalDifference(),
                TextToFloat(),
                TinInterpolation(),
                TopoColor(),
                Translate(),
                TruncateTable(),
                Union(),
                UniqueValues(),
                VariableDistanceBuffer(),
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
