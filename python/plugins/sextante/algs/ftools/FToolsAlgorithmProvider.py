# -*- coding: utf-8 -*-

"""
***************************************************************************
    FToolsAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
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
from sextante.ftools.PointsInPolygonWeighted import PointsInPolygonWeighted
from sextante.ftools.PointsInPolygonUnique import PointsInPolygonUnique

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

from PyQt4 import QtGui

from sextante.core.AlgorithmProvider import AlgorithmProvider

# analysis tools
from sextante.ftools.SumLines import SumLines
from sextante.ftools.MeanCoords import MeanCoords
from sextante.ftools.UniqueValues import UniqueValues
from sextante.ftools.PointDistance import PointDistance
from sextante.ftools.BasicStatisticsStrings import BasicStatisticsStrings
from sextante.ftools.BasicStatisticsNumbers import BasicStatisticsNumbers
from sextante.ftools.PointsInPolygon import PointsInPolygon
from sextante.ftools.LinesIntersection import LinesIntersection
from sextante.ftools.NearestNeighbourAnalysis import NearestNeighbourAnalysis

# data management tools
from sextante.ftools.ReprojectLayer import ReprojectLayer

# geometry tools
from sextante.ftools.Delaunay import Delaunay
from sextante.ftools.Centroids import Centroids
from sextante.ftools.ExtractNodes import ExtractNodes
from sextante.ftools.VoronoiPolygons import VoronoiPolygons
from sextante.ftools.LinesToPolygons import LinesToPolygons
from sextante.ftools.PolygonsToLines import PolygonsToLines
from sextante.ftools.DensifyGeometries import DensifyGeometries
from sextante.ftools.SimplifyGeometries import SimplifyGeometries
from sextante.ftools.ExportGeometryInfo import ExportGeometryInfo
from sextante.ftools.MultipartToSingleparts import MultipartToSingleparts
from sextante.ftools.SinglePartsToMultiparts import SinglePartsToMultiparts

# geoprocessing tools
from sextante.ftools.Clip import Clip
from sextante.ftools.Union import Union
from sextante.ftools.Dissolve import Dissolve
from sextante.ftools.ConvexHull import ConvexHull
from sextante.ftools.Difference import Difference
from sextante.ftools.Intersection import Intersection
from sextante.ftools.FixedDistanceBuffer import FixedDistanceBuffer
from sextante.ftools.VariableDistanceBuffer import VariableDistanceBuffer

# research tools
from sextante.ftools.ExtentFromLayer import ExtentFromLayer
from sextante.ftools.RandomSelection import RandomSelection
from sextante.ftools.SelectByLocation import SelectByLocation
from sextante.ftools.RandomSelectionWithinSubsets import RandomSelectionWithinSubsets

class FToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [SumLines(), PointsInPolygon(), PointsInPolygonWeighted(), PointsInPolygonUnique(),
                        BasicStatisticsStrings(), BasicStatisticsNumbers(), NearestNeighbourAnalysis(),
                        MeanCoords(), LinesIntersection(), UniqueValues(), PointDistance(),
                        # data management
                        ReprojectLayer(),
                        # geometry
                        ExportGeometryInfo(), Centroids(), Delaunay(), VoronoiPolygons(),
                        SimplifyGeometries(), DensifyGeometries(), MultipartToSingleparts(),
                        SinglePartsToMultiparts(), PolygonsToLines(), LinesToPolygons(),
                        ExtractNodes(),
                        # geoprocessing
                        ConvexHull(), FixedDistanceBuffer(), VariableDistanceBuffer(),
                        Dissolve(), Difference(), Intersection(), Union(), Clip(),
                        # research
                        ExtentFromLayer(), RandomSelection(), RandomSelectionWithinSubsets(),
                        SelectByLocation()
                       ]

    def getDescription(self):
        return "QGIS native algorithms"

    def getName(self):
        return "ftools"

    def getIcon(self):
        return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/ftools_logo.png")

    def _loadAlgorithms(self):
        self.algs = self.alglist

    def getSupportedOutputTableExtensions(self):
        return ["csv"]

    def supportsNonFileBasedOutput(self):
        return True
