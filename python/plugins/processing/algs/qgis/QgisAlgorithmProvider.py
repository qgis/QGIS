# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithmProvider.py
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

import os

from qgis.core import (QgsApplication,
                       QgsProcessingProvider)

from PyQt5.QtCore import QCoreApplication

from .Aggregate import Aggregate
from .BarPlot import BarPlot
from .BasicStatistics import BasicStatisticsForField
from .BoxPlot import BoxPlot
from .CheckValidity import CheckValidity
from .Climb import Climb
from .ConcaveHull import ConcaveHull
from .Datasources2Vrt import Datasources2Vrt
from .DefineProjection import DefineProjection
from .Delaunay import Delaunay
from .DeleteColumn import DeleteColumn
from .DeleteDuplicateGeometries import DeleteDuplicateGeometries
from .EliminateSelection import EliminateSelection
from .ExecuteSQL import ExecuteSQL
from .ExportGeometryInfo import ExportGeometryInfo
from .FieldPyculator import FieldsPyculator
from .FieldsCalculator import FieldsCalculator
from .FieldsMapper import FieldsMapper
from .FindProjection import FindProjection
from .GeometryConvert import GeometryConvert
from .Heatmap import Heatmap
from .HubDistanceLines import HubDistanceLines
from .HubDistancePoints import HubDistancePoints
from .HypsometricCurves import HypsometricCurves
from .IdwInterpolation import IdwInterpolation
from .ImportIntoPostGIS import ImportIntoPostGIS
from .ImportIntoSpatialite import ImportIntoSpatialite
from .KeepNBiggestParts import KeepNBiggestParts
from .KNearestConcaveHull import KNearestConcaveHull
from .LinesToPolygons import LinesToPolygons
from .MeanAndStdDevPlot import MeanAndStdDevPlot
from .MinimumBoundingGeometry import MinimumBoundingGeometry
from .NearestNeighbourAnalysis import NearestNeighbourAnalysis
from .PointDistance import PointDistance
from .PointsDisplacement import PointsDisplacement
from .PointsFromLines import PointsFromLines
from .PointsFromPolygons import PointsFromPolygons
from .PointsToPaths import PointsToPaths
from .PolarPlot import PolarPlot
from .PoleOfInaccessibility import PoleOfInaccessibility
from .Polygonize import Polygonize
from .PostGISExecuteSQL import PostGISExecuteSQL
from .PostGISExecuteAndLoadSQL import PostGISExecuteAndLoadSQL
from .RandomExtract import RandomExtract
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RandomPointsAlongLines import RandomPointsAlongLines
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygons import RandomPointsPolygons
from .RandomSelection import RandomSelection
from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from .RasterCalculator import RasterCalculator
from .RasterLayerHistogram import RasterLayerHistogram
from .RasterLayerStatistics import RasterLayerStatistics
from .RasterSampling import RasterSampling
from .RectanglesOvalsDiamondsFixed import RectanglesOvalsDiamondsFixed
from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
from .RegularPoints import RegularPoints
from .Relief import Relief
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .SetRasterStyle import SetRasterStyle
from .SetVectorStyle import SetVectorStyle
from .SnapGeometries import SnapGeometriesToLayer
from .SpatialiteExecuteSQL import SpatialiteExecuteSQL
from .SpatialJoin import SpatialJoin
from .SpatialJoinSummary import SpatialJoinSummary
from .StatisticsByCategories import StatisticsByCategories
from .TextToFloat import TextToFloat
from .TilesXYZ import TilesXYZAlgorithmDirectory, TilesXYZAlgorithmMBTiles
from .TinInterpolation import TinInterpolation
from .TopoColors import TopoColor
from .TruncateTable import TruncateTable
from .UniqueValues import UniqueValues
from .VariableDistanceBuffer import VariableDistanceBuffer
from .VectorLayerHistogram import VectorLayerHistogram
from .VectorLayerScatterplot import VectorLayerScatterplot
from .VectorLayerScatterplot3D import VectorLayerScatterplot3D
from .VectorSplit import VectorSplit
from .VoronoiPolygons import VoronoiPolygons
from .ZonalStatistics import ZonalStatistics


class QgisAlgorithmProvider(QgsProcessingProvider):
    fieldMappingParameterName = QCoreApplication.translate('Processing', 'Fields Mapper')

    def __init__(self):
        super().__init__()
        self.algs = []
        self.externalAlgs = []

    def getAlgs(self):
        algs = [Aggregate(),
                BarPlot(),
                BasicStatisticsForField(),
                BoxPlot(),
                CheckValidity(),
                Climb(),
                ConcaveHull(),
                Datasources2Vrt(),
                DefineProjection(),
                Delaunay(),
                DeleteColumn(),
                DeleteDuplicateGeometries(),
                EliminateSelection(),
                ExecuteSQL(),
                ExportGeometryInfo(),
                FieldsCalculator(),
                FieldsMapper(),
                FieldsPyculator(),
                FindProjection(),
                GeometryConvert(),
                Heatmap(),
                HubDistanceLines(),
                HubDistancePoints(),
                HypsometricCurves(),
                IdwInterpolation(),
                ImportIntoPostGIS(),
                ImportIntoSpatialite(),
                KeepNBiggestParts(),
                KNearestConcaveHull(),
                LinesToPolygons(),
                MeanAndStdDevPlot(),
                MinimumBoundingGeometry(),
                NearestNeighbourAnalysis(),
                PointDistance(),
                PointsDisplacement(),
                PointsFromLines(),
                PointsFromPolygons(),
                PointsToPaths(),
                PolarPlot(),
                PoleOfInaccessibility(),
                Polygonize(),
                PostGISExecuteSQL(),
                PostGISExecuteAndLoadSQL(),
                RandomExtract(),
                RandomExtractWithinSubsets(),
                RandomPointsAlongLines(),
                RandomPointsLayer(),
                RandomPointsPolygons(),
                RandomSelection(),
                RandomSelectionWithinSubsets(),
                RasterCalculator(),
                RasterLayerHistogram(),
                RasterLayerStatistics(),
                RasterSampling(),
                RectanglesOvalsDiamondsFixed(),
                RectanglesOvalsDiamondsVariable(),
                RegularPoints(),
                Relief(),
                SelectByAttribute(),
                SelectByExpression(),
                SetRasterStyle(),
                SetVectorStyle(),
                SnapGeometriesToLayer(),
                SpatialiteExecuteSQL(),
                SpatialJoin(),
                SpatialJoinSummary(),
                StatisticsByCategories(),
                TextToFloat(),
                TilesXYZAlgorithmDirectory(),
                TilesXYZAlgorithmMBTiles(),
                TinInterpolation(),
                TopoColor(),
                TruncateTable(),
                UniqueValues(),
                VariableDistanceBuffer(),
                VectorLayerHistogram(),
                VectorLayerScatterplot(),
                VectorLayerScatterplot3D(),
                VectorSplit(),
                VoronoiPolygons(),
                ZonalStatistics()
                ]

        return algs

    def id(self):
        return 'qgis'

    def helpId(self):
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

    def load(self):
        success = super().load()

        if success:
            self.parameterTypeFieldsMapping = FieldsMapper.ParameterFieldsMappingType()
            QgsApplication.instance().processingRegistry().addParameterType(self.parameterTypeFieldsMapping)

        return success

    def unload(self):
        super().unload()

        QgsApplication.instance().processingRegistry().removeParameterType(self.parameterTypeFieldsMapping)

    def supportsNonFileBasedOutput(self):
        return True
