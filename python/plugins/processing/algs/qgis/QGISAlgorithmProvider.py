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
from .DropMZValues import DropMZValues
from .ExtendLines import ExtendLines
from .ExtentFromLayer import ExtentFromLayer
from .ExtractNodes import ExtractNodes
from .FixGeometry import FixGeometry
from .GeometryByExpression import GeometryByExpression
from .GridPolygon import GridPolygon
from .Heatmap import Heatmap
from .Hillshade import Hillshade
from .ImportIntoPostGIS import ImportIntoPostGIS
from .ImportIntoSpatialite import ImportIntoSpatialite
from .Intersection import Intersection
from .LinesIntersection import LinesIntersection
from .LinesToPolygons import LinesToPolygons
from .MeanCoords import MeanCoords
from .Merge import Merge
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .OffsetLine import OffsetLine
from .Orthogonalize import Orthogonalize
from .PointDistance import PointDistance
from .PointOnSurface import PointOnSurface
from .PointsInPolygon import PointsInPolygon
from .PointsLayerFromTable import PointsLayerFromTable
from .PoleOfInaccessibility import PoleOfInaccessibility
from .PolygonsToLines import PolygonsToLines
from .PostGISExecuteSQL import PostGISExecuteSQL
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RandomPointsAlongLines import RandomPointsAlongLines
from .RandomPointsExtent import RandomPointsExtent
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygons import RandomPointsPolygons
from .RegularPoints import RegularPoints
from .ReverseLineDirection import ReverseLineDirection
from .Ruggedness import Ruggedness
from .SaveSelectedFeatures import SaveSelectedFeatures
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .ServiceAreaFromLayer import ServiceAreaFromLayer
from .ServiceAreaFromPoint import ServiceAreaFromPoint
from .SetMValue import SetMValue
from .SetZValue import SetZValue
from .ShortestPathLayerToPoint import ShortestPathLayerToPoint
from .ShortestPathPointToLayer import ShortestPathPointToLayer
from .ShortestPathPointToPoint import ShortestPathPointToPoint
from .SimplifyGeometries import SimplifyGeometries
from .SingleSidedBuffer import SingleSidedBuffer
from .Slope import Slope
from .Smooth import Smooth
from .SnapGeometries import SnapGeometriesToLayer
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .SumLines import SumLines
from .SymmetricalDifference import SymmetricalDifference
from .Translate import Translate
from .Union import Union
from .UniqueValues import UniqueValues
from .VectorSplit import VectorSplit
from .VoronoiPolygons import VoronoiPolygons
from .ZonalStatistics import ZonalStatistics

# from .ExtractByLocation import ExtractByLocation
# from .ExportGeometryInfo import ExportGeometryInfo
# from .SinglePartsToMultiparts import SinglePartsToMultiparts
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
# from .PointsToPaths import PointsToPaths
# from .SetVectorStyle import SetVectorStyle
# from .SetRasterStyle import SetRasterStyle
# from .SelectByAttributeSum import SelectByAttributeSum
# from .HypsometricCurves import HypsometricCurves
# from .SplitWithLines import SplitWithLines
# from .FieldsMapper import FieldsMapper
# from .Datasources2Vrt import Datasources2Vrt
# from .OrientedMinimumBoundingBox import OrientedMinimumBoundingBox
# from .SpatialIndex import SpatialIndex
# from .DefineProjection import DefineProjection
# from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
# from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
# from .MergeLines import MergeLines
# from .PointsAlongGeometry import PointsAlongGeometry
# from .Relief import Relief
# from .IdwInterpolation import IdwInterpolation
# from .TinInterpolation import TinInterpolation
# from .ExtractSpecificNodes import ExtractSpecificNodes
# from .RasterCalculator import RasterCalculator
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
        # algs = [
        #
        #         ExportGeometryInfo(),
        #         SinglePartsToMultiparts(),
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
        #         SpatialIndex(), DefineProjection(),
        #         RectanglesOvalsDiamondsVariable(),
        #         RectanglesOvalsDiamondsFixed(), MergeLines(),
        #         PointsAlongGeometry(),
        #         Relief(),
        #         IdwInterpolation(), TinInterpolation(),
        #         ExtractSpecificNodes(),
        #         RasterCalculator(),
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
                DropMZValues(),
                ExtendLines(),
                ExtentFromLayer(),
                ExtractNodes(),
                FixGeometry(),
                GeometryByExpression(),
                GridPolygon(),
                Heatmap(),
                Hillshade(),
                ImportIntoPostGIS(),
                ImportIntoSpatialite(),
                Intersection(),
                LinesIntersection(),
                LinesToPolygons(),
                MeanCoords(),
                Merge(),
                NearestNeighbourAnalysis(),
                OffsetLine(),
                Orthogonalize(),
                PointDistance(),
                PointOnSurface(),
                PointsInPolygon(),
                PointsLayerFromTable(),
                PoleOfInaccessibility(),
                PolygonsToLines(),
                PostGISExecuteSQL(),
                RandomExtract(),
                RandomExtractWithinSubsets(),
                RandomPointsAlongLines(),
                RandomPointsExtent(),
                RandomPointsLayer(),
                RandomPointsPolygons(),
                RegularPoints(),
                ReverseLineDirection(),
                Ruggedness(),
                SaveSelectedFeatures(),
                SelectByAttribute(),
                SelectByExpression(),
                ServiceAreaFromLayer(),
                ServiceAreaFromPoint(),
                SetMValue(),
                SetZValue(),
                ShortestPathLayerToPoint(),
                ShortestPathPointToLayer(),
                ShortestPathPointToPoint(),
                SimplifyGeometries(),
                SingleSidedBuffer(),
                Slope(),
                Smooth(),
                SnapGeometriesToLayer(),
                SpatialiteExecuteSQL(),
                SumLines(),
                SymmetricalDifference(),
                Translate(),
                Union(),
                UniqueValues(),
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
