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
    import matplotlib.pyplot
    hasMatplotlib = True
except:
    hasMatplotlib = False

from PyQt4.QtGui import QIcon

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.script.ScriptUtils import ScriptUtils

from RegularPoints import RegularPoints
from SymmetricalDifference import SymmetricalDifference
from VectorSplit import VectorSplit
from VectorGrid import VectorGrid
from RandomExtract import RandomExtract
from RandomExtractWithinSubsets import RandomExtractWithinSubsets
from ExtractByLocation import ExtractByLocation
from PointsInPolygon import PointsInPolygon
from PointsInPolygonUnique import PointsInPolygonUnique
from PointsInPolygonWeighted import PointsInPolygonWeighted
from SumLines import SumLines
from BasicStatisticsNumbers import BasicStatisticsNumbers
from BasicStatisticsStrings import BasicStatisticsStrings
from NearestNeighbourAnalysis import NearestNeighbourAnalysis
from LinesIntersection import LinesIntersection
from MeanCoords import MeanCoords
from PointDistance import PointDistance
from UniqueValues import UniqueValues
from ReprojectLayer import ReprojectLayer
from ExportGeometryInfo import ExportGeometryInfo
from Centroids import Centroids
from Delaunay import Delaunay
from VoronoiPolygons import VoronoiPolygons
from DensifyGeometries import DensifyGeometries
from MultipartToSingleparts import MultipartToSingleparts
from SimplifyGeometries import SimplifyGeometries
from LinesToPolygons import LinesToPolygons
from PolygonsToLines import PolygonsToLines
from SinglePartsToMultiparts import SinglePartsToMultiparts
from ExtractNodes import ExtractNodes
from ConvexHull import ConvexHull
from FixedDistanceBuffer import FixedDistanceBuffer
from VariableDistanceBuffer import VariableDistanceBuffer
from Clip import Clip
from Difference import Difference
from Dissolve import Dissolve
from Intersection import Intersection
from ExtentFromLayer import ExtentFromLayer
from RandomSelection import RandomSelection
from RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from SelectByLocation import SelectByLocation
from Union import Union
from DensifyGeometriesInterval import DensifyGeometriesInterval
from Eliminate import Eliminate
from SpatialJoin import SpatialJoin
from DeleteColumn import DeleteColumn
from DeleteHoles import DeleteHoles
from DeleteDuplicateGeometries import DeleteDuplicateGeometries
from TextToFloat import TextToFloat
from ExtractByAttribute import ExtractByAttribute
from SelectByAttribute import SelectByAttribute
from Grid import Grid
from Gridify import Gridify
from HubDistance import HubDistance
from HubLines import HubLines
from Merge import Merge
from GeometryConvert import GeometryConvert
from ConcaveHull import ConcaveHull
from Polygonize import Polygonize
from RasterLayerStatistics import RasterLayerStatistics
from StatisticsByCategories import StatisticsByCategories
from EquivalentNumField import EquivalentNumField
from AddTableField import AddTableField
from FieldsCalculator import FieldsCalculator
from SaveSelectedFeatures import SaveSelectedFeatures
from Explode import Explode
from AutoincrementalField import AutoincrementalField
from FieldPyculator import FieldsPyculator
from JoinAttributes import JoinAttributes
from CreateConstantRaster import CreateConstantRaster
from PointsLayerFromTable import PointsLayerFromTable
from PointsDisplacement import PointsDisplacement
from ZonalStatistics import ZonalStatistics
from PointsFromPolygons import PointsFromPolygons
from PointsFromLines import PointsFromLines
from RandomPointsExtent import RandomPointsExtent
from RandomPointsLayer import RandomPointsLayer
from RandomPointsPolygonsFixed import RandomPointsPolygonsFixed
from RandomPointsPolygonsVariable import RandomPointsPolygonsVariable
from RandomPointsAlongLines import RandomPointsAlongLines
from PointsToPaths import PointsToPaths
from PostGISExecuteSQL import PostGISExecuteSQL
from ImportIntoPostGIS import ImportIntoPostGIS
from SetVectorStyle import SetVectorStyle
from SetRasterStyle import SetRasterStyle
from SelectByExpression import SelectByExpression
from SelectByAttributeSum import SelectByAttributeSum
from HypsometricCurves import HypsometricCurves
from SplitLinesWithLines import SplitLinesWithLines
from FieldsMapper import FieldsMapper
from Datasources2Vrt import Datasources2Vrt
from CheckValidity import CheckValidity

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class QGISAlgorithmProvider(AlgorithmProvider):

    _icon = QIcon(os.path.join(pluginPath, 'images', 'qgis.png'))

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [SumLines(), PointsInPolygon(),
                        PointsInPolygonWeighted(), PointsInPolygonUnique(),
                        BasicStatisticsStrings(), BasicStatisticsNumbers(),
                        NearestNeighbourAnalysis(), MeanCoords(),
                        LinesIntersection(), UniqueValues(), PointDistance(),
                        ReprojectLayer(), ExportGeometryInfo(), Centroids(),
                        Delaunay(), VoronoiPolygons(), SimplifyGeometries(),
                        DensifyGeometries(), DensifyGeometriesInterval(),
                        MultipartToSingleparts(), SinglePartsToMultiparts(),
                        PolygonsToLines(), LinesToPolygons(), ExtractNodes(),
                        Eliminate(), ConvexHull(), FixedDistanceBuffer(),
                        VariableDistanceBuffer(), Dissolve(), Difference(),
                        Intersection(), Union(), Clip(), ExtentFromLayer(),
                        RandomSelection(), RandomSelectionWithinSubsets(),
                        SelectByLocation(), RandomExtract(), DeleteHoles(),
                        RandomExtractWithinSubsets(), ExtractByLocation(),
                        SpatialJoin(), RegularPoints(), SymmetricalDifference(),
                        VectorSplit(), VectorGrid(), DeleteColumn(),
                        DeleteDuplicateGeometries(), TextToFloat(),
                        ExtractByAttribute(), SelectByAttribute(), Grid(),
                        Gridify(), HubDistance(), HubLines(), Merge(),
                        GeometryConvert(), AddTableField(), FieldsCalculator(),
                        SaveSelectedFeatures(), JoinAttributes(),
                        AutoincrementalField(), Explode(), FieldsPyculator(),
                        EquivalentNumField(), PointsLayerFromTable(),
                        StatisticsByCategories(), ConcaveHull(), Polygonize(),
                        RasterLayerStatistics(), PointsDisplacement(),
                        ZonalStatistics(), PointsFromPolygons(),
                        PointsFromLines(), RandomPointsExtent(),
                        RandomPointsLayer(), RandomPointsPolygonsFixed(),
                        RandomPointsPolygonsVariable(),
                        RandomPointsAlongLines(), PointsToPaths(),
                        PostGISExecuteSQL(), ImportIntoPostGIS(),
                        SetVectorStyle(), SetRasterStyle(),
                        SelectByExpression(), HypsometricCurves(),
                        SplitLinesWithLines(), CreateConstantRaster(),
                        FieldsMapper(),SelectByAttributeSum(), Datasources2Vrt(),
                        CheckValidity()
                        ]

        if hasMatplotlib:
            from VectorLayerHistogram import VectorLayerHistogram
            from RasterLayerHistogram import RasterLayerHistogram
            from VectorLayerScatterplot import VectorLayerScatterplot
            from MeanAndStdDevPlot import MeanAndStdDevPlot
            from BarPlot import BarPlot
            from PolarPlot import PolarPlot

            self.alglist.extend([
                VectorLayerHistogram(), RasterLayerHistogram(),
                VectorLayerScatterplot(), MeanAndStdDevPlot(), BarPlot(),
                PolarPlot(),
            ])

        folder = os.path.join(os.path.dirname(__file__), 'scripts')
        scripts = ScriptUtils.loadFromFolder(folder)
        for script in scripts:
            script.allowEdit = False
        self.alglist.extend(scripts)
        for alg in self.alglist:
            alg._icon = self._icon

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)

    def unload(self):
        AlgorithmProvider.unload(self)

    def getName(self):
        return 'qgis'

    def getDescription(self):
        return self.tr('QGIS geoalgorithms')

    def getIcon(self):
        return self._icon

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def supportsNonFileBasedOutput(self):
        return True
