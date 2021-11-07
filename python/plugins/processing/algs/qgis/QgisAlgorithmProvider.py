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
                       QgsProcessingProvider,
                       QgsRuntimeProfiler)

from PyQt5.QtCore import QCoreApplication

from .BarPlot import BarPlot
from .BasicStatistics import BasicStatisticsForField
from .BoxPlot import BoxPlot
from .CheckValidity import CheckValidity
from .Climb import Climb
from .ConcaveHull import ConcaveHull
from .DefineProjection import DefineProjection
from .Delaunay import Delaunay
from .EliminateSelection import EliminateSelection
from .ExecuteSQL import ExecuteSQL
from .ExportGeometryInfo import ExportGeometryInfo
from .FieldPyculator import FieldsPyculator
from .FindProjection import FindProjection
from .GeometryConvert import GeometryConvert
from .Heatmap import Heatmap
from .HubDistanceLines import HubDistanceLines
from .HubDistancePoints import HubDistancePoints
from .HypsometricCurves import HypsometricCurves
from .IdwInterpolation import IdwInterpolation
from .ImportIntoSpatialite import ImportIntoSpatialite
from .KeepNBiggestParts import KeepNBiggestParts
from .KNearestConcaveHull import KNearestConcaveHull
from .LinesToPolygons import LinesToPolygons
from .MeanAndStdDevPlot import MeanAndStdDevPlot
from .MinimumBoundingGeometry import MinimumBoundingGeometry
from .PointDistance import PointDistance
from .PointsDisplacement import PointsDisplacement
from .PointsFromLines import PointsFromLines
from .PolarPlot import PolarPlot
from .PostGISExecuteAndLoadSQL import PostGISExecuteAndLoadSQL
from .RandomExtractWithinSubsets import RandomExtractWithinSubsets
from .RandomPointsAlongLines import RandomPointsAlongLines
from .RandomPointsLayer import RandomPointsLayer
from .RandomPointsPolygons import RandomPointsPolygons
from .RandomSelection import RandomSelection
from .RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from .RasterCalculator import RasterCalculator
from .RasterLayerHistogram import RasterLayerHistogram
from .RectanglesOvalsDiamondsVariable import RectanglesOvalsDiamondsVariable
from .RegularPoints import RegularPoints
from .Relief import Relief
from .SelectByAttribute import SelectByAttribute
from .SelectByExpression import SelectByExpression
from .SetRasterStyle import SetRasterStyle
from .SetVectorStyle import SetVectorStyle
from .SpatialJoinSummary import SpatialJoinSummary
from .StatisticsByCategories import StatisticsByCategories
from .TextToFloat import TextToFloat
from .TilesXYZ import TilesXYZAlgorithmDirectory, TilesXYZAlgorithmMBTiles
from .TinInterpolation import TinInterpolation
from .TopoColors import TopoColor
from .UniqueValues import UniqueValues
from .VariableDistanceBuffer import VariableDistanceBuffer
from .VectorLayerHistogram import VectorLayerHistogram
from .VectorLayerScatterplot import VectorLayerScatterplot
from .VectorLayerScatterplot3D import VectorLayerScatterplot3D
from .VoronoiPolygons import VoronoiPolygons


class QgisAlgorithmProvider(QgsProcessingProvider):
    fieldMappingParameterName = QCoreApplication.translate('Processing', 'Fields Mapper')

    def __init__(self):
        super().__init__()
        QgsApplication.processingRegistry().addAlgorithmAlias('qgis:rectanglesovalsdiamondsfixed', 'native:rectanglesovalsdiamonds')

    def getAlgs(self):
        algs = [BarPlot(),
                BasicStatisticsForField(),
                BoxPlot(),
                CheckValidity(),
                Climb(),
                ConcaveHull(),
                DefineProjection(),
                Delaunay(),
                EliminateSelection(),
                ExecuteSQL(),
                ExportGeometryInfo(),
                FieldsPyculator(),
                FindProjection(),
                GeometryConvert(),
                Heatmap(),
                HubDistanceLines(),
                HubDistancePoints(),
                HypsometricCurves(),
                IdwInterpolation(),
                ImportIntoSpatialite(),
                KeepNBiggestParts(),
                KNearestConcaveHull(),
                LinesToPolygons(),
                MeanAndStdDevPlot(),
                MinimumBoundingGeometry(),
                PointDistance(),
                PointsDisplacement(),
                PointsFromLines(),
                PolarPlot(),
                PostGISExecuteAndLoadSQL(),
                RandomExtractWithinSubsets(),
                RandomPointsAlongLines(),
                RandomPointsLayer(),
                RandomPointsPolygons(),
                RandomSelection(),
                RandomSelectionWithinSubsets(),
                RasterCalculator(),
                RasterLayerHistogram(),
                RectanglesOvalsDiamondsVariable(),
                RegularPoints(),
                Relief(),
                SelectByAttribute(),
                SelectByExpression(),
                SetRasterStyle(),
                SetVectorStyle(),
                SpatialJoinSummary(),
                StatisticsByCategories(),
                TextToFloat(),
                TilesXYZAlgorithmDirectory(),
                TilesXYZAlgorithmMBTiles(),
                TinInterpolation(),
                TopoColor(),
                UniqueValues(),
                VariableDistanceBuffer(),
                VectorLayerHistogram(),
                VectorLayerScatterplot(),
                VectorLayerScatterplot3D(),
                VoronoiPolygons(),
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
        for a in self.getAlgs():
            self.addAlgorithm(a)

    def load(self):
        with QgsRuntimeProfiler.profile('QGIS Python Provider'):
            success = super().load()

        return success

    def unload(self):
        super().unload()

    def supportsNonFileBasedOutput(self):
        return True
