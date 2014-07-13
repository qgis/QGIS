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
from processing.script.ScriptUtils import ScriptUtils
import os


__author__ = 'Victor Olaya'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtGui import *
from ftools.RandomExtract import RandomExtract
from ftools.RandomExtractWithinSubsets import RandomExtractWithinSubsets
from ftools.ExtractByLocation import ExtractByLocation

from processing.core.AlgorithmProvider import AlgorithmProvider
from ftools.PointsInPolygon import PointsInPolygon
from ftools.PointsInPolygonUnique import PointsInPolygonUnique
from ftools.PointsInPolygonWeighted import PointsInPolygonWeighted
from ftools.SumLines import SumLines
from ftools.BasicStatisticsNumbers import BasicStatisticsNumbers
from ftools.BasicStatisticsStrings import BasicStatisticsStrings
from ftools.NearestNeighbourAnalysis import NearestNeighbourAnalysis
from ftools.LinesIntersection import LinesIntersection
from ftools.MeanCoords import MeanCoords
from ftools.PointDistance import PointDistance
from ftools.UniqueValues import UniqueValues
from ftools.ReprojectLayer import ReprojectLayer
from ftools.ExportGeometryInfo import ExportGeometryInfo
from ftools.Centroids import Centroids
from ftools.Delaunay import Delaunay
from ftools.VoronoiPolygons import VoronoiPolygons
from ftools.DensifyGeometries import DensifyGeometries
from ftools.MultipartToSingleparts import MultipartToSingleparts
from ftools.SimplifyGeometries import SimplifyGeometries
from ftools.LinesToPolygons import LinesToPolygons
from ftools.PolygonsToLines import PolygonsToLines
from ftools.SinglePartsToMultiparts import SinglePartsToMultiparts
from ftools.ExtractNodes import ExtractNodes
from ftools.ConvexHull import ConvexHull
from ftools.FixedDistanceBuffer import FixedDistanceBuffer
from ftools.VariableDistanceBuffer import VariableDistanceBuffer
from ftools.Clip import Clip
from ftools.Difference import Difference
from ftools.Dissolve import Dissolve
from ftools.Intersection import Intersection
from ftools.ExtentFromLayer import ExtentFromLayer
from ftools.RandomSelection import RandomSelection
from ftools.RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from ftools.SelectByLocation import SelectByLocation
from ftools.Union import Union
from ftools.DensifyGeometriesInterval import DensifyGeometriesInterval
from ftools.Eliminate import Eliminate
from ftools.SpatialJoin import SpatialJoin

from mmqgisx.MMQGISXAlgorithms import *

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
# from VectorLayerHistogram import VectorLayerHistogram
# from VectorLayerScatterplot import VectorLayerScatterplot
# from MeanAndStdDevPlot import MeanAndStdDevPlot
# from BarPlot import BarPlot
# from PolarPlot import PolarPlot
# from RasterLayerHistogram import RasterLayerHistogram

import processing.resources_rc


class QGISAlgorithmProvider(AlgorithmProvider):

    _icon = QIcon(':/processing/images/qgis.png')

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
                        SelectByLocation(), RandomExtract(),
                        RandomExtractWithinSubsets(), ExtractByLocation(),
                        SpatialJoin(),
                        # ------ mmqgisx ------
                        mmqgisx_delete_columns_algorithm(),
                        mmqgisx_delete_duplicate_geometries_algorithm(),
                        mmqgisx_geometry_convert_algorithm(),
                        mmqgisx_grid_algorithm(),
                        mmqgisx_gridify_algorithm(),
                        mmqgisx_hub_distance_algorithm(),
                        mmqgisx_hub_lines_algorithm(),
                        mmqgisx_merge_algorithm(),
                        mmqgisx_select_algorithm(),
                        mmqgisx_extract_algorithm(),
                        mmqgisx_text_to_float_algorithm(),
                        # ------ native algs ------
                        AddTableField(), FieldsCalculator(),
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
                        SetVectorStyle(), SetRasterStyle(), SelectByExpression()
                        # ------ raster ------
                        # CreateConstantRaster(),
                        # ------ graphics ------
                        # VectorLayerHistogram(), VectorLayerScatterplot(),
                        # RasterLayerHistogram(), MeanAndStdDevPlot(),
                        # BarPlot(), PolarPlot()
                       ]

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
        return 'QGIS geoalgorithms'

    def getIcon(self):
        return self._icon

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def supportsNonFileBasedOutput(self):
        return True
