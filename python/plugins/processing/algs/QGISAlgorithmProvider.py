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
from processing.algs.Polygonize import Polygonize

__author__ = 'Victor Olaya'
__date__ = 'December 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtGui import *

from processing.core.AlgorithmProvider import AlgorithmProvider

from processing.algs.ftools.PointsInPolygon import PointsInPolygon
from processing.algs.ftools.PointsInPolygonUnique import PointsInPolygonUnique
from processing.algs.ftools.PointsInPolygonWeighted import PointsInPolygonWeighted
from processing.algs.ftools.SumLines import SumLines
from processing.algs.ftools.BasicStatisticsNumbers import BasicStatisticsNumbers
from processing.algs.ftools.BasicStatisticsStrings import BasicStatisticsStrings
from processing.algs.ftools.NearestNeighbourAnalysis import NearestNeighbourAnalysis
from processing.algs.ftools.LinesIntersection import LinesIntersection
from processing.algs.ftools.MeanCoords import MeanCoords
from processing.algs.ftools.PointDistance import PointDistance
from processing.algs.ftools.UniqueValues import UniqueValues
from processing.algs.ftools.ReprojectLayer import ReprojectLayer
from processing.algs.ftools.ExportGeometryInfo import ExportGeometryInfo
from processing.algs.ftools.Centroids import Centroids
from processing.algs.ftools.Delaunay import Delaunay
from processing.algs.ftools.VoronoiPolygons import VoronoiPolygons
from processing.algs.ftools.DensifyGeometries import DensifyGeometries
from processing.algs.ftools.MultipartToSingleparts import MultipartToSingleparts
from processing.algs.ftools.SimplifyGeometries import SimplifyGeometries
from processing.algs.ftools.LinesToPolygons import LinesToPolygons
from processing.algs.ftools.PolygonsToLines import PolygonsToLines
from processing.algs.ftools.SinglePartsToMultiparts import SinglePartsToMultiparts
from processing.algs.ftools.ExtractNodes import ExtractNodes
from processing.algs.ftools.ConvexHull import ConvexHull
from processing.algs.ftools.FixedDistanceBuffer import FixedDistanceBuffer
from processing.algs.ftools.VariableDistanceBuffer import VariableDistanceBuffer
from processing.algs.ftools.Clip import Clip
from processing.algs.ftools.Difference import Difference
from processing.algs.ftools.Dissolve import Dissolve
from processing.algs.ftools.Intersection import Intersection
from processing.algs.ftools.ExtentFromLayer import ExtentFromLayer
from processing.algs.ftools.RandomSelection import RandomSelection
from processing.algs.ftools.RandomSelectionWithinSubsets import RandomSelectionWithinSubsets
from processing.algs.ftools.SelectByLocation import SelectByLocation
from processing.algs.ftools.Union import Union
from processing.algs.ftools.DensifyGeometriesInterval import DensifyGeometriesInterval

from processing.algs.mmqgisx.MMQGISXAlgorithms import  (mmqgisx_delete_columns_algorithm,
    mmqgisx_delete_duplicate_geometries_algorithm,
    mmqgisx_geometry_convert_algorithm,
    mmqgisx_grid_algorithm, mmqgisx_gridify_algorithm,
    mmqgisx_hub_distance_algorithm, mmqgisx_hub_lines_algorithm,
    mmqgisx_merge_algorithm, mmqgisx_select_algorithm,
    mmqgisx_text_to_float_algorithm)

from processing.algs.RasterLayerStatistics import RasterLayerStatistics
from processing.algs.StatisticsByCategories import StatisticsByCategories
from processing.algs.EquivalentNumField import EquivalentNumField
from processing.algs.AddTableField import AddTableField
from processing.algs.FieldsCalculator import FieldsCalculator
from processing.algs.SaveSelectedFeatures import SaveSelectedFeatures
from processing.algs.Explode import Explode
from processing.algs.AutoincrementalField import AutoincrementalField
from processing.algs.FieldPyculator import FieldsPyculator
from processing.algs.JoinAttributes import JoinAttributes
from processing.algs.CreateConstantRaster import CreateConstantRaster
from processing.algs.PointsLayerFromTable import PointsLayerFromTable

from processing.algs.PointsDisplacement import PointsDisplacement
from processing.algs.ZonalStatistics import ZonalStatistics
from processing.algs.PointsFromPolygons import PointsFromPolygons
from processing.algs.PointsFromLines import PointsFromLines

#from processing.algs.VectorLayerHistogram import VectorLayerHistogram
#from processing.algs.VectorLayerScatterplot import VectorLayerScatterplot
#from processing.algs.MeanAndStdDevPlot import MeanAndStdDevPlot
#from processing.algs.BarPlot import BarPlot
#from processing.algs.PolarPlot import PolarPlot
#from processing.algs.RasterLayerHistogram import RasterLayerHistogram

import processing.resources_rc

class QGISAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [# ------ fTools ------
                        SumLines(), PointsInPolygon(), PointsInPolygonWeighted(),
                        PointsInPolygonUnique(), BasicStatisticsStrings(),
                        BasicStatisticsNumbers(), NearestNeighbourAnalysis(),
                        MeanCoords(), LinesIntersection(), UniqueValues(), PointDistance(),
                        # data management
                        ReprojectLayer(),
                        # geometry
                        ExportGeometryInfo(), Centroids(), Delaunay(), VoronoiPolygons(),
                        SimplifyGeometries(), DensifyGeometries(), DensifyGeometriesInterval(),
                        MultipartToSingleparts(), SinglePartsToMultiparts(), PolygonsToLines(),
                        LinesToPolygons(), ExtractNodes(),
                        # geoprocessing
                        ConvexHull(), FixedDistanceBuffer(), VariableDistanceBuffer(),
                        Dissolve(), Difference(), Intersection(), Union(), Clip(),
                        # research
                        ExtentFromLayer(), RandomSelection(), RandomSelectionWithinSubsets(),
                        SelectByLocation(),
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
                        mmqgisx_text_to_float_algorithm(),
                        # ------ native algs ------
                        AddTableField(), FieldsCalculator(), SaveSelectedFeatures(), JoinAttributes(),
                        AutoincrementalField(), Explode(), FieldsPyculator(), EquivalentNumField(),
                        PointsLayerFromTable(), StatisticsByCategories(), Polygonize(),
                        # ------ raster ------
                        #CreateConstantRaster(),
                        RasterLayerStatistics(),
                        # ------ graphics ------
                        #VectorLayerHistogram(), VectorLayerScatterplot(), RasterLayerHistogram(),
                        #MeanAndStdDevPlot(), BarPlot(), PolarPlot()
                        # ------ vector ------
                        PointsDisplacement(),
                        ZonalStatistics(),
                        PointsFromPolygons(),
                        PointsFromLines()
                        ]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)

    def unload(self):
        AlgorithmProvider.unload(self)

    def getName(self):
        return "qgis"

    def getDescription(self):
        return "QGIS geoalgorithms"

    def getIcon(self):
        return QIcon(":/processing/images/qgis.png")

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def supportsNonFileBasedOutput(self):
        return True
